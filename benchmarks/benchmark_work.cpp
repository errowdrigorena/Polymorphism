module benchmark_work;

import std;
import polymorphic;
import reflection_type_erasure;
import shapes;
import type_erasure;

// Defined in timer.cpp. An opaque call, so a draw() string has to exist for
// every technique, including the direct loop the compiler can see.
extern "C++" void keep_result(const char* data, unsigned long size);

// Every family checks the same five techniques:
// direct, traditional_polymorphism, type_erasure, std::polymorphic,
// and reflection_type_erasure.
//
// draw() returns std::string for shapes, type_erasure, and std::polymorphic.
// The reflection library's draw() writes to std::cout, so that row uses Poly
// of a model whose draw() returns the same string. area() returns float for
// the reflection library. shapes and Shape_TE have no numeric method, so the
// area rows for traditional_polymorphism, type_erasure, and std::polymorphic
// use a local base with the same formula. unique_ptr does not copy the derived
// object, so traditional_polymorphism copies through a virtual clone().
//
// Each function is one Google Benchmark iteration: a single pass over
// object_count values. direct keeps one concrete type in a contiguous array.
// cycle_* builds, calls draw() and area() once, and destroys.
// cycle_*_5, _50, _500, and _5000 alternate those calls that many times.
// Even counts are split in half. Five calls are three draws and two areas.

namespace {

static_assert(object_count % 3 == 0);

enum class Family { disk, box, wedge };

constexpr Family family_at(std::size_t index)
{
    return static_cast<Family>(index % 3);
}

constexpr float span_at(std::size_t index)
{
    return 0.25f + static_cast<float>(index % 97) * 0.01f;
}

std::string disk_name(float span)
{
    return span >= 1.f ? "disk" : "dot";
}

std::string box_name(float span)
{
    return span >= 1.f ? "box" : "dot";
}

std::string wedge_name(float span)
{
    return span >= 1.f ? "wedge" : "dot";
}

float disk_area(float span)
{
    return span * span;
}

float box_area(float span)
{
    return span * span * 2.f;
}

float wedge_area(float span)
{
    return span * 0.5f;
}

// Same bodies as the virtual leaves. type_erasure stores these, and the
// direct string loop calls them. They are not part of the Shape hierarchy,
// so the erased call is one function pointer.
struct ValueDisk {
    float span = 0;

    std::string draw() const
    {
        return disk_name(span);
    }

    float area() const
    {
        return disk_area(span);
    }
};

struct ValueBox {
    float span = 0;

    std::string draw() const
    {
        return box_name(span);
    }

    float area() const
    {
        return box_area(span);
    }
};

struct ValueWedge {
    float span = 0;

    std::string draw() const
    {
        return wedge_name(span);
    }

    float area() const
    {
        return wedge_area(span);
    }
};

class DrawnDisk final : public Shape {
public:
    explicit DrawnDisk(float span) : span_(span)
    {
    }

    std::string draw() const override
    {
        return disk_name(span_);
    }

private:
    float span_;
};

class DrawnBox final : public Shape {
public:
    explicit DrawnBox(float span) : span_(span)
    {
    }

    std::string draw() const override
    {
        return box_name(span_);
    }

private:
    float span_;
};

class DrawnWedge final : public Shape {
public:
    explicit DrawnWedge(float span) : span_(span)
    {
    }

    std::string draw() const override
    {
        return wedge_name(span_);
    }

private:
    float span_;
};

// reflection_type_erasure accepts any type that matches ShapeModel.
// draw() is required and unused: the library's own draw() prints.
struct ScalarDisk {
    float span = 0;

    void draw()
    {
    }

    float area()
    {
        return disk_area(span);
    }

    float perimeter()
    {
        return span + span;
    }
};

struct ScalarBox {
    float span = 0;

    void draw()
    {
    }

    float area()
    {
        return box_area(span);
    }

    float perimeter()
    {
        return span * 4.f;
    }
};

struct ScalarWedge {
    float span = 0;

    void draw()
    {
    }

    float area()
    {
        return wedge_area(span);
    }

    float perimeter()
    {
        return span * 3.f;
    }
};

// traditional_polymorphism for area(). The shapes hierarchy has no numeric method.
// The five special members stay, for the same reason as Shape.
class Area {
public:
    virtual ~Area() = default;
    Area() = default;
    Area(const Area&) = default;
    Area& operator=(const Area&) = default;
    Area(Area&&) noexcept = default;
    Area& operator=(Area&&) noexcept = default;

    virtual float area() const = 0;
};

class AreaDisk final : public Area {
public:
    explicit AreaDisk(float span) : span_(span)
    {
    }

    float area() const override
    {
        return disk_area(span_);
    }

private:
    float span_;
};

class AreaBox final : public Area {
public:
    explicit AreaBox(float span) : span_(span)
    {
    }

    float area() const override
    {
        return box_area(span_);
    }

private:
    float span_;
};

class AreaWedge final : public Area {
public:
    explicit AreaWedge(float span) : span_(span)
    {
    }

    float area() const override
    {
        return wedge_area(span_);
    }

private:
    float span_;
};

// Hand-written type erasure for area(). Shape_TE only erases draw().
class AreaErased {
public:
    AreaErased() = default;

    template <typename T>
        requires(!std::same_as<std::decay_t<T>, AreaErased>)
    AreaErased(T shape)
        : object_(new std::decay_t<T>(std::move(shape))),
          vtable_(&vtable_for<std::decay_t<T>>())
    {
    }

    AreaErased(const AreaErased&) = delete;
    AreaErased& operator=(const AreaErased&) = delete;

    AreaErased(AreaErased&& other) noexcept
        : object_(std::exchange(other.object_, nullptr)),
          vtable_(std::exchange(other.vtable_, nullptr))
    {
    }

    AreaErased& operator=(AreaErased&&) = delete;

    ~AreaErased()
    {
        reset();
    }

    float area() const
    {
        if (object_ == nullptr) {
            return 0.f;
        }
        return vtable_->area(object_);
    }

private:
    void reset() noexcept
    {
        if (object_ != nullptr) {
            vtable_->destroy(object_);
            object_ = nullptr;
            vtable_ = nullptr;
        }
    }

    struct VTable {
        float (*area)(const void*);
        void (*destroy)(void*) noexcept;
    };

    template <typename T> static const VTable& vtable_for() noexcept
    {
        static const VTable table{
            [](const void* object) -> float { return static_cast<const T*>(object)->area(); },
            [](void* object) noexcept { delete static_cast<T*>(object); },
        };
        return table;
    }

    void* object_ = nullptr;
    const VTable* vtable_ = nullptr;
};

// traditional_polymorphism copy. unique_ptr does not clone the derived object,
// so the copy goes through a virtual clone().
class CloneShape {
public:
    virtual ~CloneShape() = default;
    CloneShape() = default;
    CloneShape(const CloneShape&) = default;
    CloneShape& operator=(const CloneShape&) = default;
    CloneShape(CloneShape&&) noexcept = default;
    CloneShape& operator=(CloneShape&&) noexcept = default;

    virtual std::unique_ptr<CloneShape> clone() const = 0;
};

class CloneDisk final : public CloneShape {
public:
    explicit CloneDisk(float span) : span_(span)
    {
    }

    std::unique_ptr<CloneShape> clone() const override
    {
        return std::make_unique<CloneDisk>(span_);
    }

private:
    float span_;
};

class CloneBox final : public CloneShape {
public:
    explicit CloneBox(float span) : span_(span)
    {
    }

    std::unique_ptr<CloneShape> clone() const override
    {
        return std::make_unique<CloneBox>(span_);
    }

private:
    float span_;
};

class CloneWedge final : public CloneShape {
public:
    explicit CloneWedge(float span) : span_(span)
    {
    }

    std::unique_ptr<CloneShape> clone() const override
    {
        return std::make_unique<CloneWedge>(span_);
    }

private:
    float span_;
};

// Virtual base for traditional_polymorphism and for std::polymorphic.
// unique_ptr owns it in the traditional row.
class CycleShape {
public:
    virtual ~CycleShape() = default;
    CycleShape() = default;
    CycleShape(const CycleShape&) = default;
    CycleShape& operator=(const CycleShape&) = default;
    CycleShape(CycleShape&&) noexcept = default;
    CycleShape& operator=(CycleShape&&) noexcept = default;

    virtual std::string draw() const = 0;
    virtual float area() const = 0;
};

class CycleDisk final : public CycleShape {
public:
    explicit CycleDisk(float span) : span_(span)
    {
    }

    std::string draw() const override
    {
        return disk_name(span_);
    }

    float area() const override
    {
        return disk_area(span_);
    }

private:
    float span_;
};

class CycleBox final : public CycleShape {
public:
    explicit CycleBox(float span) : span_(span)
    {
    }

    std::string draw() const override
    {
        return box_name(span_);
    }

    float area() const override
    {
        return box_area(span_);
    }

private:
    float span_;
};

class CycleWedge final : public CycleShape {
public:
    explicit CycleWedge(float span) : span_(span)
    {
    }

    std::string draw() const override
    {
        return wedge_name(span_);
    }

    float area() const override
    {
        return wedge_area(span_);
    }

private:
    float span_;
};

// Hand-written type erasure. Shape_TE only erases draw(), so this handle adds
// area() and still deletes the object through the vtable.
class CycleErased {
public:
    CycleErased() = default;

    template <typename T>
        requires(!std::same_as<std::decay_t<T>, CycleErased>)
    CycleErased(T shape)
        : object_(new std::decay_t<T>(std::move(shape))),
          vtable_(&vtable_for<std::decay_t<T>>())
    {
    }

    CycleErased(const CycleErased&) = delete;
    CycleErased& operator=(const CycleErased&) = delete;

    CycleErased(CycleErased&& other) noexcept
        : object_(std::exchange(other.object_, nullptr)),
          vtable_(std::exchange(other.vtable_, nullptr))
    {
    }

    CycleErased& operator=(CycleErased&&) = delete;

    ~CycleErased()
    {
        reset();
    }

    std::string draw() const
    {
        if (object_ == nullptr) {
            return "<empty>";
        }
        return vtable_->draw(object_);
    }

    float area() const
    {
        if (object_ == nullptr) {
            return 0.f;
        }
        return vtable_->area(object_);
    }

private:
    void reset() noexcept
    {
        if (object_ != nullptr) {
            vtable_->destroy(object_);
            object_ = nullptr;
            vtable_ = nullptr;
        }
    }

    struct VTable {
        std::string (*draw)(const void*);
        float (*area)(const void*);
        void (*destroy)(void*) noexcept;
    };

    template <typename T> static const VTable& vtable_for() noexcept
    {
        static const VTable table{
            [](const void* object) -> std::string {
                return static_cast<const T*>(object)->draw();
            },
            [](const void* object) -> float { return static_cast<const T*>(object)->area(); },
            [](void* object) noexcept { delete static_cast<T*>(object); },
        };
        return table;
    }

    void* object_ = nullptr;
    const VTable* vtable_ = nullptr;
};

std::vector<ValueDisk> value_disks;
std::vector<ValueBox> value_boxes;
std::vector<ValueWedge> value_wedges;
std::vector<std::unique_ptr<Shape>> drawn;
std::vector<Shape_TE> erased;
std::vector<PolyShape> polys;
std::vector<ScalarDisk> scalar_disks;
std::vector<ScalarBox> scalar_boxes;
std::vector<ScalarWedge> scalar_wedges;
std::vector<std::unique_ptr<Area>> areas;
std::vector<AnyShape> reflected;
std::vector<AreaErased> area_erased;
std::vector<std::polymorphic<Area>> area_polys;
std::vector<std::unique_ptr<CloneShape>> clone_sources;

template <typename AddDisk, typename AddBox, typename AddWedge>
void for_each_object(AddDisk add_disk, AddBox add_box, AddWedge add_wedge)
{
    for (std::size_t index = 0; index < object_count; ++index) {
        const float span = span_at(index);
        switch (family_at(index)) {
        case Family::disk:
            add_disk(span);
            break;
        case Family::box:
            add_box(span);
            break;
        case Family::wedge:
            add_wedge(span);
            break;
        }
    }
}

unsigned long long consume_text(const std::string& text)
{
    keep_result(text.data(), static_cast<unsigned long>(text.size()));
    return static_cast<unsigned long long>(text.size()) + static_cast<unsigned char>(text.front());
}

unsigned long long consume_float(float value)
{
    return std::bit_cast<std::uint32_t>(value);
}

} // namespace

// reflection_type_erasure for draw(). ShapeModel::draw() is void and prints,
// so this model returns the same string the other draw rows return.
// The consteval block stays outside the anonymous namespace: define_aggregate
// specializes VTable in this namespace.
struct DrawModel {
    virtual ~DrawModel() = default;

    virtual std::string draw() = 0;
};

consteval
{
    define_model<DrawModel>();
}

namespace {
std::vector<Poly<DrawModel>> draw_reflected;
}

// reflection_type_erasure for a full cycle. ShapeModel::draw() is void, so this
// model returns the name and the area.
struct CycleModel {
    virtual ~CycleModel() = default;

    virtual std::string draw() = 0;
    virtual float area() = 0;
};

consteval
{
    define_model<CycleModel>();
}

unsigned long long sweep_draw_direct()
{
    unsigned long long sink = 0;
    for (const ValueDisk& disk : value_disks) {
        sink += consume_text(disk.draw());
    }
    for (const ValueBox& box : value_boxes) {
        sink += consume_text(box.draw());
    }
    for (const ValueWedge& wedge : value_wedges) {
        sink += consume_text(wedge.draw());
    }
    return sink;
}

unsigned long long sweep_draw_traditional_polymorphism()
{
    unsigned long long sink = 0;
    for (const std::unique_ptr<Shape>& shape : drawn) {
        sink += consume_text(shape->draw());
    }
    return sink;
}

unsigned long long sweep_draw_type_erasure()
{
    unsigned long long sink = 0;
    for (const Shape_TE& shape : erased) {
        sink += consume_text(shape.draw());
    }
    return sink;
}

unsigned long long sweep_draw_std_polymorphic()
{
    unsigned long long sink = 0;
    for (const PolyShape& shape : polys) {
        sink += consume_text(shape->draw());
    }
    return sink;
}

unsigned long long sweep_draw_reflection_type_erasure()
{
    unsigned long long sink = 0;
    for (const Poly<DrawModel>& shape : draw_reflected) {
        sink += consume_text(shape.draw());
    }
    return sink;
}

unsigned long long sweep_area_direct()
{
    unsigned long long sink = 0;
    for (ScalarDisk& disk : scalar_disks) {
        sink += consume_float(disk.area());
    }
    for (ScalarBox& box : scalar_boxes) {
        sink += consume_float(box.area());
    }
    for (ScalarWedge& wedge : scalar_wedges) {
        sink += consume_float(wedge.area());
    }
    return sink;
}

unsigned long long sweep_area_traditional_polymorphism()
{
    unsigned long long sink = 0;
    for (const std::unique_ptr<Area>& shape : areas) {
        sink += consume_float(shape->area());
    }
    return sink;
}

unsigned long long sweep_area_type_erasure()
{
    unsigned long long sink = 0;
    for (const AreaErased& shape : area_erased) {
        sink += consume_float(shape.area());
    }
    return sink;
}

unsigned long long sweep_area_std_polymorphic()
{
    unsigned long long sink = 0;
    for (const std::polymorphic<Area>& shape : area_polys) {
        sink += consume_float(shape->area());
    }
    return sink;
}

unsigned long long sweep_area_reflection_type_erasure()
{
    unsigned long long sink = 0;
    for (AnyShape& shape : reflected) {
        sink += consume_float(shape.area());
    }
    return sink;
}

static void keep_bytes(const void* data, std::size_t bytes)
{
    keep_result(static_cast<const char*>(data), static_cast<unsigned long>(bytes));
}

unsigned long long copy_direct()
{
    auto disks = value_disks;
    auto boxes = value_boxes;
    auto wedges = value_wedges;
    keep_bytes(disks.data(), disks.size() * sizeof(ValueDisk));
    keep_bytes(boxes.data(), boxes.size() * sizeof(ValueBox));
    keep_bytes(wedges.data(), wedges.size() * sizeof(ValueWedge));
    return disks.size() + boxes.size() + wedges.size();
}

unsigned long long copy_traditional_polymorphism()
{
    std::vector<std::unique_ptr<CloneShape>> copy;
    copy.reserve(clone_sources.size());
    for (const std::unique_ptr<CloneShape>& shape : clone_sources) {
        copy.push_back(shape->clone());
    }
    return copy.size();
}

unsigned long long copy_type_erasure()
{
    auto copy = erased;
    return copy.size();
}

unsigned long long copy_std_polymorphic()
{
    auto copy = polys;
    return copy.size();
}

unsigned long long copy_reflection_type_erasure()
{
    auto copy = reflected;
    return copy.size();
}

unsigned long long build_direct()
{
    std::vector<ValueDisk> disks;
    std::vector<ValueBox> boxes;
    std::vector<ValueWedge> wedges;
    disks.reserve(object_count / 3);
    boxes.reserve(object_count / 3);
    wedges.reserve(object_count / 3);
    for_each_object([&](float span) { disks.push_back(ValueDisk{span}); },
                    [&](float span) { boxes.push_back(ValueBox{span}); },
                    [&](float span) { wedges.push_back(ValueWedge{span}); });
    keep_bytes(disks.data(), disks.size() * sizeof(ValueDisk));
    keep_bytes(boxes.data(), boxes.size() * sizeof(ValueBox));
    keep_bytes(wedges.data(), wedges.size() * sizeof(ValueWedge));
    return disks.size() + boxes.size() + wedges.size();
}

unsigned long long build_traditional_polymorphism()
{
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.push_back(std::make_unique<DrawnDisk>(span)); },
                    [&](float span) { shapes.push_back(std::make_unique<DrawnBox>(span)); },
                    [&](float span) { shapes.push_back(std::make_unique<DrawnWedge>(span)); });
    return shapes.size();
}

unsigned long long build_type_erasure()
{
    std::vector<Shape_TE> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.emplace_back(ValueDisk{span}); },
                    [&](float span) { shapes.emplace_back(ValueBox{span}); },
                    [&](float span) { shapes.emplace_back(ValueWedge{span}); });
    return shapes.size();
}

unsigned long long build_std_polymorphic()
{
    std::vector<PolyShape> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.emplace_back(std::in_place_type<DrawnDisk>, span); },
                    [&](float span) { shapes.emplace_back(std::in_place_type<DrawnBox>, span); },
                    [&](float span) { shapes.emplace_back(std::in_place_type<DrawnWedge>, span); });
    return shapes.size();
}

unsigned long long build_reflection_type_erasure()
{
    std::vector<AnyShape> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.emplace_back(ScalarDisk{span}); },
                    [&](float span) { shapes.emplace_back(ScalarBox{span}); },
                    [&](float span) { shapes.emplace_back(ScalarWedge{span}); });
    return shapes.size();
}

unsigned long long cycle_direct()
{
    std::vector<ValueDisk> disks;
    std::vector<ValueBox> boxes;
    std::vector<ValueWedge> wedges;
    disks.reserve(object_count / 3);
    boxes.reserve(object_count / 3);
    wedges.reserve(object_count / 3);
    for_each_object([&](float span) { disks.push_back(ValueDisk{span}); },
                    [&](float span) { boxes.push_back(ValueBox{span}); },
                    [&](float span) { wedges.push_back(ValueWedge{span}); });
    keep_bytes(disks.data(), disks.size() * sizeof(ValueDisk));
    keep_bytes(boxes.data(), boxes.size() * sizeof(ValueBox));
    keep_bytes(wedges.data(), wedges.size() * sizeof(ValueWedge));

    unsigned long long sink = 0;
    for (const ValueDisk& disk : disks) {
        sink += consume_text(disk.draw());
        sink += consume_float(disk.area());
    }
    for (const ValueBox& box : boxes) {
        sink += consume_text(box.draw());
        sink += consume_float(box.area());
    }
    for (const ValueWedge& wedge : wedges) {
        sink += consume_text(wedge.draw());
        sink += consume_float(wedge.area());
    }
    return sink;
}

unsigned long long cycle_traditional_polymorphism()
{
    std::vector<std::unique_ptr<CycleShape>> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.push_back(std::make_unique<CycleDisk>(span)); },
                    [&](float span) { shapes.push_back(std::make_unique<CycleBox>(span)); },
                    [&](float span) { shapes.push_back(std::make_unique<CycleWedge>(span)); });

    unsigned long long sink = 0;
    for (const std::unique_ptr<CycleShape>& shape : shapes) {
        sink += consume_text(shape->draw());
        sink += consume_float(shape->area());
    }
    return sink;
}

unsigned long long cycle_type_erasure()
{
    std::vector<CycleErased> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.emplace_back(ValueDisk{span}); },
                    [&](float span) { shapes.emplace_back(ValueBox{span}); },
                    [&](float span) { shapes.emplace_back(ValueWedge{span}); });

    unsigned long long sink = 0;
    for (const CycleErased& shape : shapes) {
        sink += consume_text(shape.draw());
        sink += consume_float(shape.area());
    }
    return sink;
}

unsigned long long cycle_std_polymorphic()
{
    std::vector<std::polymorphic<CycleShape>> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.emplace_back(std::in_place_type<CycleDisk>, span); },
                    [&](float span) { shapes.emplace_back(std::in_place_type<CycleBox>, span); },
                    [&](float span) { shapes.emplace_back(std::in_place_type<CycleWedge>, span); });

    unsigned long long sink = 0;
    for (const std::polymorphic<CycleShape>& shape : shapes) {
        sink += consume_text(shape->draw());
        sink += consume_float(shape->area());
    }
    return sink;
}

unsigned long long cycle_reflection_type_erasure()
{
    std::vector<Poly<CycleModel>> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.emplace_back(ValueDisk{span}); },
                    [&](float span) { shapes.emplace_back(ValueBox{span}); },
                    [&](float span) { shapes.emplace_back(ValueWedge{span}); });

    unsigned long long sink = 0;
    for (const Poly<CycleModel>& shape : shapes) {
        sink += consume_text(shape.draw());
        sink += consume_float(shape.area());
    }
    return sink;
}

// Even steps call draw(), odd steps call area(). The object dies with the vector.
template <int Operations>
unsigned long long alternate_draw_area(auto&& draw, auto&& area)
{
    unsigned long long sink = 0;
    for (int step = 0; step < Operations; ++step) {
        if (step % 2 == 0) {
            sink += consume_text(draw());
        }
        else {
            sink += consume_float(area());
        }
    }
    return sink;
}

template <int Operations>
unsigned long long cycle_direct_ops()
{
    std::vector<ValueDisk> disks;
    std::vector<ValueBox> boxes;
    std::vector<ValueWedge> wedges;
    disks.reserve(object_count / 3);
    boxes.reserve(object_count / 3);
    wedges.reserve(object_count / 3);
    for_each_object([&](float span) { disks.push_back(ValueDisk{span}); },
                    [&](float span) { boxes.push_back(ValueBox{span}); },
                    [&](float span) { wedges.push_back(ValueWedge{span}); });
    keep_bytes(disks.data(), disks.size() * sizeof(ValueDisk));
    keep_bytes(boxes.data(), boxes.size() * sizeof(ValueBox));
    keep_bytes(wedges.data(), wedges.size() * sizeof(ValueWedge));

    unsigned long long sink = 0;
    for (const ValueDisk& disk : disks) {
        sink += alternate_draw_area<Operations>([&] { return disk.draw(); },
                                                [&] { return disk.area(); });
    }
    for (const ValueBox& box : boxes) {
        sink += alternate_draw_area<Operations>([&] { return box.draw(); },
                                                [&] { return box.area(); });
    }
    for (const ValueWedge& wedge : wedges) {
        sink += alternate_draw_area<Operations>([&] { return wedge.draw(); },
                                                [&] { return wedge.area(); });
    }
    return sink;
}

template <int Operations>
unsigned long long cycle_traditional_polymorphism_ops()
{
    std::vector<std::unique_ptr<CycleShape>> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.push_back(std::make_unique<CycleDisk>(span)); },
                    [&](float span) { shapes.push_back(std::make_unique<CycleBox>(span)); },
                    [&](float span) { shapes.push_back(std::make_unique<CycleWedge>(span)); });

    unsigned long long sink = 0;
    for (const std::unique_ptr<CycleShape>& shape : shapes) {
        sink += alternate_draw_area<Operations>([&] { return shape->draw(); },
                                                [&] { return shape->area(); });
    }
    return sink;
}

template <int Operations>
unsigned long long cycle_type_erasure_ops()
{
    std::vector<CycleErased> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.emplace_back(ValueDisk{span}); },
                    [&](float span) { shapes.emplace_back(ValueBox{span}); },
                    [&](float span) { shapes.emplace_back(ValueWedge{span}); });

    unsigned long long sink = 0;
    for (const CycleErased& shape : shapes) {
        sink += alternate_draw_area<Operations>([&] { return shape.draw(); },
                                                [&] { return shape.area(); });
    }
    return sink;
}

template <int Operations>
unsigned long long cycle_std_polymorphic_ops()
{
    std::vector<std::polymorphic<CycleShape>> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.emplace_back(std::in_place_type<CycleDisk>, span); },
                    [&](float span) { shapes.emplace_back(std::in_place_type<CycleBox>, span); },
                    [&](float span) { shapes.emplace_back(std::in_place_type<CycleWedge>, span); });

    unsigned long long sink = 0;
    for (const std::polymorphic<CycleShape>& shape : shapes) {
        sink += alternate_draw_area<Operations>([&] { return shape->draw(); },
                                                [&] { return shape->area(); });
    }
    return sink;
}

template <int Operations>
unsigned long long cycle_reflection_type_erasure_ops()
{
    std::vector<Poly<CycleModel>> shapes;
    shapes.reserve(object_count);
    for_each_object([&](float span) { shapes.emplace_back(ValueDisk{span}); },
                    [&](float span) { shapes.emplace_back(ValueBox{span}); },
                    [&](float span) { shapes.emplace_back(ValueWedge{span}); });

    unsigned long long sink = 0;
    for (const Poly<CycleModel>& shape : shapes) {
        sink += alternate_draw_area<Operations>([&] { return shape.draw(); },
                                                [&] { return shape.area(); });
    }
    return sink;
}

unsigned long long cycle_direct_5()
{
    return cycle_direct_ops<5>();
}
unsigned long long cycle_traditional_polymorphism_5()
{
    return cycle_traditional_polymorphism_ops<5>();
}
unsigned long long cycle_type_erasure_5()
{
    return cycle_type_erasure_ops<5>();
}
unsigned long long cycle_std_polymorphic_5()
{
    return cycle_std_polymorphic_ops<5>();
}
unsigned long long cycle_reflection_type_erasure_5()
{
    return cycle_reflection_type_erasure_ops<5>();
}

unsigned long long cycle_direct_50()
{
    return cycle_direct_ops<50>();
}
unsigned long long cycle_traditional_polymorphism_50()
{
    return cycle_traditional_polymorphism_ops<50>();
}
unsigned long long cycle_type_erasure_50()
{
    return cycle_type_erasure_ops<50>();
}
unsigned long long cycle_std_polymorphic_50()
{
    return cycle_std_polymorphic_ops<50>();
}
unsigned long long cycle_reflection_type_erasure_50()
{
    return cycle_reflection_type_erasure_ops<50>();
}

unsigned long long cycle_direct_500()
{
    return cycle_direct_ops<500>();
}
unsigned long long cycle_traditional_polymorphism_500()
{
    return cycle_traditional_polymorphism_ops<500>();
}
unsigned long long cycle_type_erasure_500()
{
    return cycle_type_erasure_ops<500>();
}
unsigned long long cycle_std_polymorphic_500()
{
    return cycle_std_polymorphic_ops<500>();
}
unsigned long long cycle_reflection_type_erasure_500()
{
    return cycle_reflection_type_erasure_ops<500>();
}

unsigned long long cycle_direct_5000()
{
    return cycle_direct_ops<5000>();
}
unsigned long long cycle_traditional_polymorphism_5000()
{
    return cycle_traditional_polymorphism_ops<5000>();
}
unsigned long long cycle_type_erasure_5000()
{
    return cycle_type_erasure_ops<5000>();
}
unsigned long long cycle_std_polymorphic_5000()
{
    return cycle_std_polymorphic_ops<5000>();
}
unsigned long long cycle_reflection_type_erasure_5000()
{
    return cycle_reflection_type_erasure_ops<5000>();
}

static constexpr unsigned long bytes_of(std::size_t bytes)
{
    return static_cast<unsigned long>(bytes);
}

void prepare_benchmarks()
{
    value_disks.clear();
    value_boxes.clear();
    value_wedges.clear();
    value_disks.reserve(object_count / 3);
    value_boxes.reserve(object_count / 3);
    value_wedges.reserve(object_count / 3);
    for_each_object([&](float span) { value_disks.push_back(ValueDisk{span}); },
                    [&](float span) { value_boxes.push_back(ValueBox{span}); },
                    [&](float span) { value_wedges.push_back(ValueWedge{span}); });

    drawn.clear();
    drawn.reserve(object_count);
    for_each_object([&](float span) { drawn.push_back(std::make_unique<DrawnDisk>(span)); },
                    [&](float span) { drawn.push_back(std::make_unique<DrawnBox>(span)); },
                    [&](float span) { drawn.push_back(std::make_unique<DrawnWedge>(span)); });

    erased.clear();
    erased.reserve(object_count);
    for_each_object([&](float span) { erased.emplace_back(ValueDisk{span}); },
                    [&](float span) { erased.emplace_back(ValueBox{span}); },
                    [&](float span) { erased.emplace_back(ValueWedge{span}); });

    polys.clear();
    polys.reserve(object_count);
    for_each_object([&](float span) { polys.emplace_back(std::in_place_type<DrawnDisk>, span); },
                    [&](float span) { polys.emplace_back(std::in_place_type<DrawnBox>, span); },
                    [&](float span) { polys.emplace_back(std::in_place_type<DrawnWedge>, span); });

    scalar_disks.clear();
    scalar_boxes.clear();
    scalar_wedges.clear();
    scalar_disks.reserve(object_count / 3);
    scalar_boxes.reserve(object_count / 3);
    scalar_wedges.reserve(object_count / 3);
    for_each_object([&](float span) { scalar_disks.push_back(ScalarDisk{span}); },
                    [&](float span) { scalar_boxes.push_back(ScalarBox{span}); },
                    [&](float span) { scalar_wedges.push_back(ScalarWedge{span}); });

    areas.clear();
    areas.reserve(object_count);
    for_each_object([&](float span) { areas.push_back(std::make_unique<AreaDisk>(span)); },
                    [&](float span) { areas.push_back(std::make_unique<AreaBox>(span)); },
                    [&](float span) { areas.push_back(std::make_unique<AreaWedge>(span)); });

    reflected.clear();
    reflected.reserve(object_count);
    for_each_object([&](float span) { reflected.emplace_back(ScalarDisk{span}); },
                    [&](float span) { reflected.emplace_back(ScalarBox{span}); },
                    [&](float span) { reflected.emplace_back(ScalarWedge{span}); });

    draw_reflected.clear();
    draw_reflected.reserve(object_count);
    for_each_object([&](float span) { draw_reflected.emplace_back(ValueDisk{span}); },
                    [&](float span) { draw_reflected.emplace_back(ValueBox{span}); },
                    [&](float span) { draw_reflected.emplace_back(ValueWedge{span}); });

    area_erased.clear();
    area_erased.reserve(object_count);
    for_each_object([&](float span) { area_erased.emplace_back(ValueDisk{span}); },
                    [&](float span) { area_erased.emplace_back(ValueBox{span}); },
                    [&](float span) { area_erased.emplace_back(ValueWedge{span}); });

    area_polys.clear();
    area_polys.reserve(object_count);
    for_each_object(
        [&](float span) { area_polys.emplace_back(std::in_place_type<AreaDisk>, span); },
        [&](float span) { area_polys.emplace_back(std::in_place_type<AreaBox>, span); },
        [&](float span) { area_polys.emplace_back(std::in_place_type<AreaWedge>, span); });

    clone_sources.clear();
    clone_sources.reserve(object_count);
    for_each_object(
        [&](float span) { clone_sources.push_back(std::make_unique<CloneDisk>(span)); },
        [&](float span) { clone_sources.push_back(std::make_unique<CloneBox>(span)); },
        [&](float span) { clone_sources.push_back(std::make_unique<CloneWedge>(span)); });
}

ApproachBytes draw_bytes()
{
    return ApproachBytes{
        .direct = bytes_of(sizeof(ValueDisk)),
        .traditional_polymorphism = bytes_of(sizeof(std::unique_ptr<Shape>)),
        .type_erasure = bytes_of(sizeof(Shape_TE)),
        .std_polymorphic = bytes_of(sizeof(PolyShape)),
        .reflection_type_erasure = bytes_of(sizeof(Poly<DrawModel>)),
    };
}

ApproachBytes area_bytes()
{
    return ApproachBytes{
        .direct = bytes_of(sizeof(ScalarDisk)),
        .traditional_polymorphism = bytes_of(sizeof(std::unique_ptr<Area>)),
        .type_erasure = bytes_of(sizeof(AreaErased)),
        .std_polymorphic = bytes_of(sizeof(std::polymorphic<Area>)),
        .reflection_type_erasure = bytes_of(sizeof(AnyShape)),
    };
}

ApproachBytes copy_bytes()
{
    return ApproachBytes{
        .direct = bytes_of(sizeof(ValueDisk)),
        .traditional_polymorphism = bytes_of(sizeof(std::unique_ptr<CloneShape>)),
        .type_erasure = bytes_of(sizeof(Shape_TE)),
        .std_polymorphic = bytes_of(sizeof(PolyShape)),
        .reflection_type_erasure = bytes_of(sizeof(AnyShape)),
    };
}

ApproachBytes build_bytes()
{
    return ApproachBytes{
        .direct = bytes_of(sizeof(ValueDisk)),
        .traditional_polymorphism = bytes_of(sizeof(std::unique_ptr<Shape>)),
        .type_erasure = bytes_of(sizeof(Shape_TE)),
        .std_polymorphic = bytes_of(sizeof(PolyShape)),
        .reflection_type_erasure = bytes_of(sizeof(AnyShape)),
    };
}

ApproachBytes cycle_bytes()
{
    return ApproachBytes{
        .direct = bytes_of(sizeof(ValueDisk)),
        .traditional_polymorphism = bytes_of(sizeof(std::unique_ptr<CycleShape>)),
        .type_erasure = bytes_of(sizeof(CycleErased)),
        .std_polymorphic = bytes_of(sizeof(std::polymorphic<CycleShape>)),
        .reflection_type_erasure = bytes_of(sizeof(Poly<CycleModel>)),
    };
}
