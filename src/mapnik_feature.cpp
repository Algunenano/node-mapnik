
#include "utils.hpp"
#include "mapnik_feature.hpp"
#include "mapnik_geometry.hpp"

// mapnik
#include <mapnik/version.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/json/feature_parser.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/util/feature_to_geojson.hpp>

Nan::Persistent<FunctionTemplate> Feature::constructor;

/**
 * A single geographic feature, with geometry and properties. This is
 * typically derived from data by a datasource, but can be manually
 * created.
 *
 * @name mapnik.Feature
 * @class
 */
void Feature::Initialize(Handle<Object> target) {

    Nan::HandleScope scope;

    Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(Feature::New);
    lcons->InstanceTemplate()->SetInternalFieldCount(1);
    lcons->SetClassName(Nan::New("Feature").ToLocalChecked());

    Nan::SetPrototypeMethod(lcons, "id", id);
    Nan::SetPrototypeMethod(lcons, "extent", extent);
    Nan::SetPrototypeMethod(lcons, "attributes", attributes);
    Nan::SetPrototypeMethod(lcons, "geometry", geometry);
    Nan::SetPrototypeMethod(lcons, "toJSON", toJSON);

    Nan::SetMethod(lcons->GetFunction(),
                    "fromJSON",
                    Feature::fromJSON);

    target->Set(Nan::New("Feature").ToLocalChecked(),lcons->GetFunction());
    constructor.Reset(lcons);
}

Feature::Feature(mapnik::feature_ptr f) :
    Nan::ObjectWrap(),
    this_(f) {}

Feature::Feature(int id) :
    Nan::ObjectWrap(),
    this_() {
    // TODO - accept/require context object to reused
    ctx_ = std::make_shared<mapnik::context_type>();
    this_ = mapnik::feature_factory::create(ctx_,id);
}

Feature::~Feature()
{
}

NAN_METHOD(Feature::New)
{
    if (!info.IsConstructCall())
    {
        Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
        return;
    }

    if (info[0]->IsExternal())
    {
        Local<External> ext = info[0].As<External>();
        void* ptr = ext->Value();
        Feature* f =  static_cast<Feature*>(ptr);
        f->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        return;
    }

    // TODO - expose mapnik.Context

    if (info.Length() > 1 || info.Length() < 1 || !info[0]->IsNumber()) {
        Nan::ThrowTypeError("requires one argument: an integer feature id");
        return;
    }

    Feature* f = new Feature(info[0]->IntegerValue());
    f->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

/**
 * @memberof mapnik.Feature
 * @static
 * @name fromJSON
 * @param {string} geojson string
 *
 * Create a feature from a GeoJSON representation.
 */
NAN_METHOD(Feature::fromJSON)
{
    if (info.Length() < 1 || !info[0]->IsString()) {
        Nan::ThrowTypeError("requires one argument: a string representing a GeoJSON feature");
        return;
    }
    std::string json = TOSTR(info[0]);
    try
    {
        mapnik::feature_ptr f(mapnik::feature_factory::create(std::make_shared<mapnik::context_type>(),1));
        if (!mapnik::json::from_geojson(json,*f))
        {
            Nan::ThrowError("Failed to read GeoJSON");
            return;
        }
        Feature* feat = new Feature(f);
        Handle<Value> ext = Nan::New<External>(feat);
        info.GetReturnValue().Set(Nan::New(constructor)->GetFunction()->NewInstance(1, &ext));
    }
    catch (std::exception const& ex)
    {
        Nan::ThrowError(ex.what());
        return;
    }
}

Local<Value> Feature::NewInstance(mapnik::feature_ptr f_ptr)
{
    Nan::EscapableHandleScope scope;
    Feature* f = new Feature(f_ptr);
    Handle<Value> ext = Nan::New<External>(f);
    return scope.Escape(Nan::New(constructor)->GetFunction()->NewInstance(1, &ext));
}

/**
 * @memberof mapnik.Feature
 * @name id
 * @instance
 * @returns {number} id the feature's internal id
 */
NAN_METHOD(Feature::id)
{
    Feature* fp = Nan::ObjectWrap::Unwrap<Feature>(info.Holder());
    info.GetReturnValue().Set(Nan::New<Number>(fp->get()->id()));
}

/**
 * Get the feature's extent
 *
 * @name extent
 * @memberof mapnik.Feature
 * @instance
 * @returns {Array<number>} extent [minx, miny, maxx, maxy] order feature extent.
 */
NAN_METHOD(Feature::extent)
{
    Feature* fp = Nan::ObjectWrap::Unwrap<Feature>(info.Holder());
    Local<Array> a = Nan::New<Array>(4);
    mapnik::box2d<double> const& e = fp->get()->envelope();
    a->Set(0, Nan::New<Number>(e.minx()));
    a->Set(1, Nan::New<Number>(e.miny()));
    a->Set(2, Nan::New<Number>(e.maxx()));
    a->Set(3, Nan::New<Number>(e.maxy()));

    info.GetReturnValue().Set(a);
}

/**
 * Get the feature's attributes as an object.
 *
 * @name attributes
 * @memberof mapnik.Feature
 * @instance
 * @returns {Object} attributes
 */
NAN_METHOD(Feature::attributes)
{
    Feature* fp = Nan::ObjectWrap::Unwrap<Feature>(info.Holder());
    Local<Object> feat = Nan::New<Object>();
    mapnik::feature_ptr feature = fp->get();
    mapnik::feature_impl::iterator itr = feature->begin();
    mapnik::feature_impl::iterator end = feature->end();
    for ( ;itr!=end; ++itr)
    {
        feat->Set(Nan::New<String>(std::get<0>(*itr)).ToLocalChecked(), 
                  mapnik::util::apply_visitor(node_mapnik::value_converter(), std::get<1>(*itr))
        );
    }
    info.GetReturnValue().Set(feat);
}


/**
 * Get the feature's attributes as a Mapnik geometry.
 *
 * @name geometry
 * @memberof mapnik.Feature
 * @instance
 * @returns {mapnik.Geometry} geometry
 */
NAN_METHOD(Feature::geometry)
{
    Feature* fp = Nan::ObjectWrap::Unwrap<Feature>(info.Holder());
    info.GetReturnValue().Set(Geometry::NewInstance(fp->get()));
}

/**
 * Generate and return a GeoJSON representation of this feature
 *
 * @instance
 * @name toJSON
 * @memberof mapnik.Feature
 * @returns {string} geojson Feature object in stringified GeoJSON
 */
NAN_METHOD(Feature::toJSON)
{
    Feature* fp = Nan::ObjectWrap::Unwrap<Feature>(info.Holder());
    std::string json;
    if (!mapnik::util::to_geojson(json, *(fp->get())))
    {
        /* LCOV_EXCL_START */
        Nan::ThrowError("Failed to generate GeoJSON");
        return;
        /* LCOV_EXCL_END */
    }
    info.GetReturnValue().Set(Nan::New<String>(json).ToLocalChecked());
}

