#ifndef LUADATA_ROOT_OBJECT_H_
#define LUADATA_ROOT_OBJECT_H_

#include <lua.hpp>
#include <memory>

#include "luadata/object.h"

namespace luadata {

enum class Error {
  // All is well
  kOkay,

  // Lower-level Lua loading, parsing and executing errors
  kUnknownLuaError,
  kSyntaxError,
  kFileReadError,
  kOutOfMemory,
  kLuaRuntimeError,

  // Errors about badly-formed data
  kEmpty,
  kTooManyReturnValues,
  kReturnValueNotATable,
};

static std::string GetErrorString(Error err);

class RootObject : public Object {
 public:
  struct LoadResult {
    std::unique_ptr<RootObject> data;
    Error error;
  };
  static LoadResult LoadString(const std::string& lua_text,
                               lua_State* lua_state = nullptr);
  static LoadResult LoadFile(const std::string& file_path,
                             lua_State* lua_state = nullptr);

  lua_State* lua_state() const { return lua_state_; }

  ~RootObject() override;

 private:
  static LoadResult LoadInternal(const std::string& source,
                                 bool from_file,
                                 lua_State* lua_state);

  RootObject(lua_State* lua_state, bool own_lua_state);
  std::unique_ptr<Object> AddObject(int table_idx);
  void RemoveObject(Object* object);
  void GrowLuaStackIfNeeded() const;

  friend class Object;
  lua_State* lua_state_;
  bool own_lua_state_;
  Object* last_;
  mutable int lua_stack_space_;
};

}  // namespace luadata

#endif  // LUADATA_ROOT_OBJECT_H_