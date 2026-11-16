// implemented() has a body. define_model throws while the call is still being
// evaluated, and catching that exception here keeps this file compiling.
import std;
import reflection_type_erasure;

struct ImplementedModel {
    virtual ~ImplementedModel() = default;

    virtual float value() = 0;

    virtual float implemented()
    {
        return 1.0f;
    }
};

template <typename Model> consteval bool accepts_model()
{
    try {
        define_model<Model>();
        return true;
    }
    catch (const std::meta::exception&) {
        return false;
    }
}

static_assert(!accepts_model<ImplementedModel>());

bool implemented_model_is_rejected()
{
    return !accepts_model<ImplementedModel>();
}
