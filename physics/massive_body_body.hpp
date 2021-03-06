﻿#pragma once

#include "physics/massive_body.hpp"

#include <string>

#include "base/macros.hpp"
#include "geometry/frame.hpp"
#include "glog/logging.h"
#include "google/protobuf/descriptor.h"
#include "physics/oblate_body.hpp"
#include "quantities/constants.hpp"
#include "serialization/geometry.pb.h"

namespace principia {

using constants::GravitationalConstant;
using geometry::Frame;
using geometry::ReadFrameFromMessage;

namespace physics {

inline MassiveBody::MassiveBody(
    GravitationalParameter const& gravitational_parameter)
    : gravitational_parameter_(gravitational_parameter),
      mass_(gravitational_parameter / GravitationalConstant) {
  CHECK_NE(gravitational_parameter, GravitationalParameter())
      << "Massive body cannot have zero gravitational parameter";
}

inline MassiveBody::MassiveBody(Mass const& mass)
    : gravitational_parameter_(mass * GravitationalConstant),
      mass_(mass) {
  CHECK_NE(mass, Mass())
      << "Massive body cannot have zero mass";
}

inline GravitationalParameter const&
MassiveBody::gravitational_parameter() const {
  return gravitational_parameter_;
}

inline Mass const& MassiveBody::mass() const {
  return mass_;
}

inline bool MassiveBody::is_massless() const {
  return false;
}

inline bool MassiveBody::is_oblate() const {
  return false;
}

inline void MassiveBody::WriteToMessage(
    not_null<serialization::Body*> const message) const {
  WriteToMessage(message->mutable_massive_body());
}

inline void MassiveBody::WriteToMessage(
    not_null<serialization::MassiveBody*> const message) const {
  gravitational_parameter_.WriteToMessage(
      message->mutable_gravitational_parameter());
}

inline not_null<std::unique_ptr<MassiveBody>> MassiveBody::ReadFromMessage(
    serialization::Body const& message) {
  CHECK(message.has_massive_body());
  return ReadFromMessage(message.massive_body());
}

// This macro is a bit ugly, but trust me, it's better than the alternatives.
#define OBLATE_BODY_TAG_VALUE_CASE(value)                                      \
  case serialization::Frame::value:                                            \
    return OblateBody<                                                         \
                Frame<Tag, serialization::Frame::value, true>>::               \
                ReadFromMessage(message)

inline not_null<std::unique_ptr<MassiveBody>> MassiveBody::ReadFromMessage(
    serialization::MassiveBody const& message) {
  if (message.HasExtension(serialization::OblateBody::oblate_body)) {
    serialization::OblateBody const& extension =
        message.GetExtension(serialization::OblateBody::oblate_body);

    const google::protobuf::EnumValueDescriptor* enum_value_descriptor;
    bool is_inertial;
    ReadFrameFromMessage(extension.frame(),
                         &enum_value_descriptor,
                         &is_inertial);
    CHECK(is_inertial);

    const google::protobuf::EnumDescriptor* enum_descriptor =
        enum_value_descriptor->type();
    {
      using Tag = serialization::Frame::PluginTag;
      if (enum_descriptor == google::protobuf::GetEnumDescriptor<Tag>()) {
        switch (static_cast<Tag>(enum_value_descriptor->number())) {
          OBLATE_BODY_TAG_VALUE_CASE(ALICE_SUN);
          OBLATE_BODY_TAG_VALUE_CASE(ALICE_WORLD);
          OBLATE_BODY_TAG_VALUE_CASE(BARYCENTRIC);
          OBLATE_BODY_TAG_VALUE_CASE(OLD_BARYCENTRIC);
          OBLATE_BODY_TAG_VALUE_CASE(RENDERING);
          OBLATE_BODY_TAG_VALUE_CASE(WORLD);
          OBLATE_BODY_TAG_VALUE_CASE(WORLD_SUN);
        }
      }
    }
    {
      using Tag = serialization::Frame::SolarSystemTag;
      if (enum_descriptor == google::protobuf::GetEnumDescriptor<Tag>()) {
        switch (static_cast<Tag>(enum_value_descriptor->number())) {
          OBLATE_BODY_TAG_VALUE_CASE(ICRF_J2000_ECLIPTIC);
          OBLATE_BODY_TAG_VALUE_CASE(ICRF_J2000_EQUATOR);
        }
      }
    }
    {
      using Tag = serialization::Frame::TestTag;
      if (enum_descriptor == google::protobuf::GetEnumDescriptor<Tag>()) {
        switch (static_cast<Tag>(enum_value_descriptor->number())) {
          OBLATE_BODY_TAG_VALUE_CASE(TEST);
          OBLATE_BODY_TAG_VALUE_CASE(TEST1);
          OBLATE_BODY_TAG_VALUE_CASE(TEST2);
          OBLATE_BODY_TAG_VALUE_CASE(FROM);
          OBLATE_BODY_TAG_VALUE_CASE(THROUGH);
          OBLATE_BODY_TAG_VALUE_CASE(TO);
        }
      }
    }
    LOG(FATAL) << enum_descriptor->name();
    base::noreturn();
  } else {
    return std::make_unique<MassiveBody>(
        GravitationalParameter::ReadFromMessage(
            message.gravitational_parameter()));
  }
}

#undef OBLATE_BODY_TAG_VALUE_CASE

}  // namespace physics
}  // namespace principia
