// The model declares methods of several arities, including a reference parameter.
// import std: this GCC instantiates define_model in the importer, and that
// instantiation looks up std::vector.
import std;
import reflection_type_erasure;

struct ParameterModel {
    virtual ~ParameterModel() = default;

    virtual float scale(float factor, float bias) = 0;
    virtual float blend(float first, float second, float weight) = 0;
    virtual float accumulate(float& total, float step) = 0;
};

struct Linear {
    float origin = 3.0f;

    float scale(float factor, float bias)
    {
        return origin * factor + bias;
    }

    float blend(float first, float second, float weight)
    {
        return first * (1.0f - weight) + second * weight;
    }

    float accumulate(float& total, float step)
    {
        total += step;
        return total;
    }
};

consteval
{
    define_model<ParameterModel>();
}

float scaled_parameter()
{
    return Poly<ParameterModel>{Linear{}}.scale(2.0f, 0.5f);
}

float blended_parameter()
{
    return Poly<ParameterModel>{Linear{}}.blend(1.0f, 5.0f, 0.25f);
}

float accumulated_parameter()
{
    Poly<ParameterModel> value{Linear{}};
    float total = 10.0f;
    value.accumulate(total, 4.0f);
    return total;
}
