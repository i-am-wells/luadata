#include <iostream>

#include "luadata/root_object.h"

namespace luadata {
namespace {

const char kOkay[] = "all is well";
const char kUnknownLuaError[] = "Unknown Lua error.";
const char kSyntaxError[] = "Lua syntax error.";
const char kFileReadError[] = "Lua reported a file read error.";
const char kOutOfMemory[] = "Lua tried and failed to allocate memory.";
const char kLuaRuntimeError[] = "Lua reported a runtime error.";
const char kEmpty[] =
    "The script didn't return anything. Make sure it looks like 'return {}'.";
const char kTooManyReturnValues[] =
    "The script returned multiple values, but only one is expected.";
const char kReturnValueNotATable[] =
    "Expected a table, but the script returned some other type.";

Error ConvertLuaError(int error_from_lua) {
  switch (error_from_lua) {
    case LUA_OK:
      return Error::kOkay;
    case LUA_ERRSYNTAX:
      return Error::kSyntaxError;
    case LUA_ERRMEM:
      return Error::kOutOfMemory;
    case LUA_ERRFILE:
      return Error::kFileReadError;
    case LUA_ERRRUN:
      return Error::kLuaRuntimeError;
    default:
      return Error::kUnknownLuaError;
  }
}

}  // namespace

std::string GetErrorString(Error err) {
  switch (err) {
    case Error::kOkay:
      return kOkay;
    case Error::kUnknownLuaError:
      return kUnknownLuaError;
    case Error::kSyntaxError:
      return kSyntaxError;
    case Error::kFileReadError:
      return kFileReadError;
    case Error::kOutOfMemory:
      return kOutOfMemory;
    case Error::kLuaRuntimeError:
      return kLuaRuntimeError;
    case Error::kEmpty:
      return kEmpty;
    case Error::kTooManyReturnValues:
      return kTooManyReturnValues;
    case Error::kReturnValueNotATable:
      return kReturnValueNotATable;
  }
  return kUnknownLuaError;
}

// static
RootObject::LoadResult RootObject::LoadString(const std::string& lua_text,
                                              lua_State* lua_state) {
  return LoadInternal(lua_text, /*from_file=*/false, lua_state);
}

// static
RootObject::LoadResult RootObject::LoadFile(const std::string& file_path,
                                            lua_State* lua_state) {
  return LoadInternal(file_path, /*from_file=*/true, lua_state);
}

// static
RootObject::LoadResult RootObject::LoadInternal(const std::string& source,
                                                bool from_file,
                                                lua_State* lua_state) {
  bool own_lua_state = false;
  if (!lua_state) {
    // TODO: sandbox?
    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

    own_lua_state = true;
  }

  int load_status;
  if (from_file) {
    load_status = luaL_loadfile(lua_state, source.c_str());
  } else {
    load_status =
        luaL_loadbuffer(lua_state, source.data(), source.size(), "string");
  }

  if (load_status != LUA_OK)
    return {nullptr, ConvertLuaError(load_status)};

  int pcall_status = lua_pcall(lua_state, /*nargs=*/0, /*nresults=*/1,
                               /*message_handler_stack_index=*/0);
  if (pcall_status != LUA_OK) {
    if (lua_isstring(lua_state, 1)) {
      std::cerr << "Failed to load ";
      if (from_file)
        std::cerr << source;
      else
        std::cerr << "Lua string";
      std::cerr << ". Lua error: " << lua_tostring(lua_state, 1) << '\n';
    }
    return {nullptr, ConvertLuaError(pcall_status)};
  }

  // A well-formed input program returns exactly one Lua table
  int return_count = lua_gettop(lua_state);
  if (return_count == 0)
    return {nullptr, Error::kEmpty};

  if (return_count > 1)
    return {nullptr, Error::kTooManyReturnValues};

  if (!lua_istable(lua_state, 1))
    return {nullptr, Error::kReturnValueNotATable};

  return {std::unique_ptr<RootObject>(new RootObject(lua_state, own_lua_state)),
          Error::kOkay};
}

RootObject::RootObject(lua_State* lua_state, bool own_lua_state)
    : Object(1, this, nullptr),
      lua_state_(lua_state),
      own_lua_state_(own_lua_state),
      last_(this) {}

RootObject::~RootObject() {
  if (own_lua_state_)
    lua_close(lua_state_);
}

std::unique_ptr<Object> RootObject::AddObject(int table_idx) {
  GrowLuaStackIfNeeded();

  auto result = std::unique_ptr<Object>(new Object(table_idx, this, last_));
  last_->next_ = result.get();
  last_ = result.get();
  return std::move(result);
}

void RootObject::RemoveObject(Object* object) {
  if (object == last_) {
    lua_pop(lua_state_, 1);
    last_ = last_->prev_;
    if (last_)
      last_->next_ = nullptr;
  } else {
    // Swap object with last_ and remove
    int top_idx = lua_gettop(lua_state_);
    lua_copy(lua_state_, top_idx, object->own_table_index_);
    lua_pop(lua_state_, 1);

    Object* orig_last = last_;
    last_ = last_->prev_;
    last_->next_ = nullptr;

    orig_last->prev_ = object->prev_;
    orig_last->prev_->next_ = orig_last;
    orig_last->next_ = object->next_;
    orig_last->next_->prev_ = orig_last;
  }
}

void RootObject::GrowLuaStackIfNeeded() const {
  if (lua_gettop(lua_state_) + 2 >= lua_stack_space_) {
    lua_stack_space_ *= 2;
    lua_checkstack(lua_state_, lua_stack_space_);
  }
}

}  // namespace luadata