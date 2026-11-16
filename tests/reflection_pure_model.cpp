// A model whose methods are all pure virtual. The destructor is a special
// member, so the defaulted virtual destructor is not part of the vtable.
// import std: this GCC instantiates define_model in the importer, and that
// instantiation looks up std::vector.
import std;
import reflection_type_erasure;

struct PureModel {
    virtual ~PureModel() = default;

    virtual float value() = 0;
};

struct FixedValue {
    float value()
    {
        return 7.0f;
    }
};

consteval
{
    define_model<PureModel>();
}

float pure_model_value()
{
    return Poly<PureModel>{FixedValue{}}.value();
}
