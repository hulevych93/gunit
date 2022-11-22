﻿#pragma once

#include "params.h"

#include "utils/numeric_cast.h"

#include <optional>
#include <type_traits>

namespace gunit {
namespace recorder {

class CodeSink {
 public:
  virtual ~CodeSink() = default;
  virtual void addLocal(const char* name, std::string&& code) = 0;
};

struct UserDataParam {
 public:
  virtual ~UserDataParam() = default;
  virtual bool equals(const UserDataParam& other) const = 0;
  virtual Argument generate(CodeSink& sink) const = 0;
  virtual UserDataParamPtr clone() const = 0;
};

inline bool operator==(const UserDataParamPtr& left,
                       const UserDataParamPtr& right) {
  return (*left).equals(*right);
}

inline bool operator!=(const UserDataParamPtr& left,
                       const UserDataParamPtr& right) {
  return !operator==(left, right);
}

template <typename Type>
Argument generateImpl(const Type&, CodeSink& sink);

namespace details {

template <typename Type>
struct UserDataParamImpl final : public UserDataParam {
 public:
  using ParamType = typename std::decay_t<Type>;

 public:
  UserDataParamImpl(Type&& param) : _param(std::forward<Type>(param)) {}

  ~UserDataParamImpl() override = default;

  bool equals(const UserDataParam& other) const override {
    if (const auto otherUserData =
            dynamic_cast<const UserDataParamImpl*>(&other)) {
      return _param == otherUserData->_param;
    }
    return false;
  }

  Argument generate(CodeSink& sink) const override {
    return generateImpl(_param, sink);
  }

  UserDataParamPtr clone() const override {
    return std::make_shared<UserDataParamImpl>(*this);
  }

 private:
  ParamType _param;
};

template <typename Type>
std::shared_ptr<UserDataParam> makeUserData(Type&& arg) {
  return std::make_shared<UserDataParamImpl<Type>>(std::forward<Type>(arg));
}

template <typename Type>
constexpr bool isUserData =
    utils::isTypeNotInTypeList<std::decay_t<Type>, GeneratorTypesList>();

//` The `makeParamImpl` functions are supposed to construct correct parameter
// types ` from wide list of possible input types. Below are sfinae functions
// for each group ` of input param types.

//` The `UserData` is made from input data of enum and class types, which are
// not included ` in the generators primitive types list.
template <typename Type,
          typename std::enable_if_t<isUserData<Type> &&
                                        (std::is_enum_v<std::decay_t<Type>> ||
                                         std::is_class_v<std::decay_t<Type>>),
                                    void*> = nullptr>
Param makeParamImpl(Type&& arg) {
  return makeUserData(std::forward<Type>(arg));
}

//` For every param type from which std::string is constructible, but not the
// std::string itself. ` Example, the `const char*` type is copied into
// std::string, and becomes one of the primitive types.
template <typename Type,
          typename std::enable_if_t<
              !std::is_same_v<std::decay_t<Type>, std::string> &&
                  std::is_constructible_v<std::string, std::decay_t<Type>>,
              void*> = nullptr>
Param makeParamImpl(Type arg) {
  return std::string{std::move(arg)};
}

//` Every floating-point type is converted into `float` with gunit::numCast.
//` The `BadNumCastError` can be thrown.
template <
    typename Type,
    typename std::enable_if_t<!std::is_same_v<std::decay_t<Type>, float> &&
                                  std::is_floating_point_v<std::decay_t<Type>>,
                              void*> = nullptr>
Param makeParamImpl(const Type arg) {
  return gunit::numCast<float>(arg);
}

//` Every integer type is converted into `int` with gunit::numCast.
//` The `BadNumCastError` can be thrown.
template <
    typename Type,
    typename std::enable_if_t<!std::is_same_v<std::decay_t<Type>, int> &&
                                  !std::is_same_v<std::decay_t<Type>, bool> &&
                                  std::is_integral_v<std::decay_t<Type>>,
                              void*> = nullptr>
Param makeParamImpl(const Type arg) {
  return gunit::numCast<int>(arg);
}

//` The boolean overload is processed with this function.
//` And also generation tags like `Nil`, `DocumentRange`, etc.
template <typename Type,
          typename std::enable_if_t<!isUserData<Type>, void*> = nullptr>
Param makeParamImpl(Type&& arg) {
  return std::move(arg);
}

}  // namespace details

//` The `makeParam` functions are supposed to overload the helper templates,
//` such as boost::optional, etc.

template <typename Type>
Param makeParam(Type&& arg) {
  return details::makeParamImpl(std::forward<Type>(arg));
}

template <typename Type>
Param makeParam(std::optional<Type> arg) {
  if (arg) {
    return details::makeParamImpl(std::move(arg.get()));
  }
  return Nil{};
}

}  // namespace recorder
}  // namespace gunit
