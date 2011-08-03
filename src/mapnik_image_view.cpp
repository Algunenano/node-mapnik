
// node
#include <node_buffer.h>
#include <node_version.h>

// mapnik
#include <mapnik/image_util.hpp>
#include <mapnik/graphics.hpp>
//#include <mapnik/image_reader.hpp>

// boost
#include <boost/make_shared.hpp>

#include "mapnik_image_view.hpp"
#include "utils.hpp"

// std
#include <exception>

Persistent<FunctionTemplate> ImageView::constructor;

void ImageView::Initialize(Handle<Object> target) {

    HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(ImageView::New));
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(String::NewSymbol("ImageView"));

    NODE_SET_PROTOTYPE_METHOD(constructor, "encodeSync", encodeSync);
    NODE_SET_PROTOTYPE_METHOD(constructor, "encode", encode);
    NODE_SET_PROTOTYPE_METHOD(constructor, "save", save);
    NODE_SET_PROTOTYPE_METHOD(constructor, "width", width);
    NODE_SET_PROTOTYPE_METHOD(constructor, "height", height);

    target->Set(String::NewSymbol("ImageView"),constructor->GetFunction());
}


ImageView::ImageView(image_view_ptr this_) :
  ObjectWrap(),
  this_(this_) {}

ImageView::~ImageView()
{
}

Handle<Value> ImageView::New(const Arguments& args)
{
    HandleScope scope;
    if (!args.IsConstructCall())
        return ThrowException(String::New("Cannot call constructor as function, you need to use 'new' keyword"));

    if (args[0]->IsExternal())
    {
        //std::clog << "image view external!\n";
        Local<External> ext = Local<External>::Cast(args[0]);
        void* ptr = ext->Value();
        ImageView* im =  static_cast<ImageView*>(ptr);
        im->Wrap(args.This());
        return args.This();
    } else {
            return ThrowException(String::New("Cannot create this object from Javascript"));
    }
    return Undefined();
}

Handle<Value> ImageView::New(boost::shared_ptr<mapnik::image_32> image_ptr,
    unsigned x,
    unsigned y,
    unsigned w,
    unsigned h
    )
{
    HandleScope scope;
    typedef boost::shared_ptr<mapnik::image_view<mapnik::image_data_32> > im_view_ptr_type;
    im_view_ptr_type image_view_ptr = boost::make_shared<mapnik::image_view<mapnik::image_data_32> >(image_ptr->get_view(x,y,w,h));
    ImageView* imv = new ImageView(image_view_ptr);
    Handle<Value> ext = External::New(imv);
    Handle<Object> obj = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(obj);

}

Handle<Value> ImageView::width(const Arguments& args)
{
    HandleScope scope;

    ImageView* im = ObjectWrap::Unwrap<ImageView>(args.This());
    return scope.Close(Integer::New(im->get()->width()));
}

Handle<Value> ImageView::height(const Arguments& args)
{
    HandleScope scope;

    ImageView* im = ObjectWrap::Unwrap<ImageView>(args.This());
    return scope.Close(Integer::New(im->get()->height()));
}


Handle<Value> ImageView::encodeSync(const Arguments& args)
{
    HandleScope scope;

    ImageView* im = ObjectWrap::Unwrap<ImageView>(args.This());
    
    std::string format = "png8"; //default to 256 colors
    
    // accept custom format
    if (args.Length() >= 1){
        if (!args[0]->IsString())
          return ThrowException(Exception::TypeError(
            String::New("first arg, 'format' must be a string")));
        format = TOSTR(args[0]);
    }
    
    try {
        std::string s = mapnik::save_to_string(*(im->this_), format);
        #if NODE_VERSION_AT_LEAST(0,3,0)
        node::Buffer *retbuf = Buffer::New((char*)s.data(),s.size());
        #else
        node::Buffer *retbuf = Buffer::New(s.size());
        memcpy(retbuf->data(), s.data(), s.size());
        #endif
        return scope.Close(retbuf->handle_);
    }
    catch (std::exception & ex)
    {
        return ThrowException(Exception::Error(
          String::New(ex.what())));
    }
    catch (...)
    {
        return ThrowException(Exception::Error(
          String::New("unknown exception happened when encoding image: please file bug report")));    
    }
}

typedef struct {
    ImageView* im;
    boost::shared_ptr<mapnik::image_view<mapnik::image_data_32> > image;
    std::string format;
    bool error;
    std::string error_name;
    Persistent<Function> cb;
    std::string result;
} encode_image_baton_t;


Handle<Value> ImageView::encode(const Arguments& args)
{
    HandleScope scope;

    ImageView* im = ObjectWrap::Unwrap<ImageView>(args.This());

    std::string format = "png8"; //default to 256 colors

    // accept custom format
    if (args.Length() >= 1){
        if (!args[0]->IsString())
          return ThrowException(Exception::TypeError(
            String::New("first arg, 'format' must be a string")));
        format = TOSTR(args[0]);
    }

    // ensure callback is a function
    Local<Value> callback = args[args.Length()-1];
    if (!args[args.Length()-1]->IsFunction())
        return ThrowException(Exception::TypeError(
                  String::New("last argument must be a callback function")));

    encode_image_baton_t *closure = new encode_image_baton_t();

    closure->im = im;
    closure->image = im->this_;
    closure->format = format;
    closure->error = false;
    closure->cb = Persistent<Function>::New(Handle<Function>::Cast(callback));
    eio_custom(EIO_Encode, EIO_PRI_DEFAULT, EIO_AfterEncode, closure);
    ev_ref(EV_DEFAULT_UC);
    im->Ref();

    return Undefined();
}

int ImageView::EIO_Encode(eio_req* req)
{
    encode_image_baton_t *closure = static_cast<encode_image_baton_t *>(req->data);

    try {
        closure->result = mapnik::save_to_string(*(closure->image), closure->format);
    }
    catch (std::exception & ex)
    {
        closure->error = true;
        closure->error_name = ex.what();
    }
    catch (...)
    {
        closure->error = true;
        closure->error_name = "unknown exception happened when encoding image: please file bug report";
    }
    return 0;
}

int ImageView::EIO_AfterEncode(eio_req* req)
{
    HandleScope scope;

    encode_image_baton_t *closure = static_cast<encode_image_baton_t *>(req->data);
    ev_unref(EV_DEFAULT_UC);

    TryCatch try_catch;

    if (closure->error) {
        Local<Value> argv[1] = { Exception::Error(String::New(closure->error_name.c_str())) };
        closure->cb->Call(Context::GetCurrent()->Global(), 1, argv);
    } else {
        #if NODE_VERSION_AT_LEAST(0,3,0)
        node::Buffer *retbuf = Buffer::New((char*)closure->result.data(),closure->result.size());
        #else
        node::Buffer *retbuf = Buffer::New(closure->result.size());
        memcpy(retbuf->data(), closure->result.data(), closure->result.size());
        #endif
        Local<Value> argv[2] = { Local<Value>::New(Null()), Local<Value>::New(retbuf->handle_) };
        closure->cb->Call(Context::GetCurrent()->Global(), 2, argv);
    }

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

    closure->im->Unref();
    closure->cb.Dispose();
    delete closure;
    return 0;
}


//  help compiler see correct definition
void (*save_view)(mapnik::image_view<mapnik::image_data_32> const&, std::string const&) = mapnik::save_to_file;

Handle<Value> ImageView::save(const Arguments& args)
{
    HandleScope scope;

    if (!args.Length() >= 1 || !args[0]->IsString()){
        return ThrowException(Exception::TypeError(
          String::New("filename required")));
    }
    
    std::string filename = TOSTR(args[0]);
    
    std::string format("");
    
    if (args.Length() >= 2) {
        if (!args[1]->IsString())
          return ThrowException(Exception::TypeError(
            String::New("both 'filename' and 'format' arguments must be strings")));

        format = mapnik::guess_type(TOSTR(args[1]));
        if (format == "<unknown>") {
            std::ostringstream s("");
            s << "unknown output extension for: " << filename << "\n";
            return ThrowException(Exception::Error(
                String::New(s.str().c_str())));
        }
    }

    ImageView* im = ObjectWrap::Unwrap<ImageView>(args.This());
    try
    {
        save_view(*im->get(),filename);
    }
    catch (const std::exception & ex)
    {
        return ThrowException(Exception::Error(
          String::New(ex.what())));
    }
    catch (...)
    {
        return ThrowException(Exception::TypeError(
          String::New("unknown exception happened while saving an image, please submit a bug report")));
    }
    
    return Undefined();
}

            
