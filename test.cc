// Unit tests for LuaData.
// Compile and run:
// ```
//     clang++ -O0 -g -std=c++17 test.cc luadata.cc -o test -llua
//     ./test
// ```

#include <cassert>
#include <iostream>

#include "object.h"
#include "root_object.h"

namespace {

using luadata::Object;
using luadata::RootObject;

void TestLoadString() {
  auto [object, error] = RootObject::LoadString("`");
  assert(!object);
  assert(error == luadata::Error::kSyntaxError);

  auto [object1, error1] = RootObject::LoadString("return 1");
  assert(!object1);
  assert(error1 == luadata::Error::kReturnValueNotATable);

  auto [object2, error2] = RootObject::LoadString("");
  assert(!object2);
  // assert(error2 == luadata::Error::kEmpty);

  auto [object3, error3] = RootObject::LoadString("return {}, {}");
  assert(object3.get());

  auto [object4, error4] = RootObject::LoadString("return {}");
  assert(object4.get());
  assert(error4 == luadata::Error::kOkay);
}

void TestGetData() {
  auto [object, error] = RootObject::LoadString(
      "return {"
      "somestring = 1,"
      "[42] = 2,"
      "[true] = 3,"
      "[3.14] = 4,"
      "}");

  assert(object.get());

  assert(1 == object->GetInt("somestring"));
  assert(2 == object->GetInt(42));
  assert(3 == object->GetInt(true));
  assert(4 == object->GetInt(3.14));
}

void TestGetObject() {
  auto [object, error] = RootObject::LoadString(
      "return {"
      "a = {x = 'some string'},"
      "b = {y = 100},"
      "}");

  assert(object.get());

  auto object_a = object->GetObject("a");
  assert(object_a);
  assert(std::string("some string") == object_a->GetString("x"));
}

}  // namespace

int main(int argc, char** argv) {
  TestLoadString();
  TestGetData();
  TestGetObject();
  std::cerr << "Ran all tests.\n";
  return 0;
}
