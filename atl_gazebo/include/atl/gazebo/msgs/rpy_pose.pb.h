// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: rpy_pose.proto

#ifndef PROTOBUF_rpy_5fpose_2eproto__INCLUDED
#define PROTOBUF_rpy_5fpose_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace atl_msgs {
namespace msgs {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_rpy_5fpose_2eproto();
void protobuf_AssignDesc_rpy_5fpose_2eproto();
void protobuf_ShutdownFile_rpy_5fpose_2eproto();

class RPYPose;

// ===================================================================

class RPYPose : public ::google::protobuf::Message {
 public:
  RPYPose();
  virtual ~RPYPose();

  RPYPose(const RPYPose& from);

  inline RPYPose& operator=(const RPYPose& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RPYPose& default_instance();

  void Swap(RPYPose* other);

  // implements Message ----------------------------------------------

  RPYPose* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RPYPose& from);
  void MergeFrom(const RPYPose& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required double x = 1;
  inline bool has_x() const;
  inline void clear_x();
  static const int kXFieldNumber = 1;
  inline double x() const;
  inline void set_x(double value);

  // required double y = 2;
  inline bool has_y() const;
  inline void clear_y();
  static const int kYFieldNumber = 2;
  inline double y() const;
  inline void set_y(double value);

  // required double z = 3;
  inline bool has_z() const;
  inline void clear_z();
  static const int kZFieldNumber = 3;
  inline double z() const;
  inline void set_z(double value);

  // required double roll = 4;
  inline bool has_roll() const;
  inline void clear_roll();
  static const int kRollFieldNumber = 4;
  inline double roll() const;
  inline void set_roll(double value);

  // required double pitch = 5;
  inline bool has_pitch() const;
  inline void clear_pitch();
  static const int kPitchFieldNumber = 5;
  inline double pitch() const;
  inline void set_pitch(double value);

  // required double yaw = 6;
  inline bool has_yaw() const;
  inline void clear_yaw();
  static const int kYawFieldNumber = 6;
  inline double yaw() const;
  inline void set_yaw(double value);

  // @@protoc_insertion_point(class_scope:atl_msgs.msgs.RPYPose)
 private:
  inline void set_has_x();
  inline void clear_has_x();
  inline void set_has_y();
  inline void clear_has_y();
  inline void set_has_z();
  inline void clear_has_z();
  inline void set_has_roll();
  inline void clear_has_roll();
  inline void set_has_pitch();
  inline void clear_has_pitch();
  inline void set_has_yaw();
  inline void clear_has_yaw();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double x_;
  double y_;
  double z_;
  double roll_;
  double pitch_;
  double yaw_;
  friend void  protobuf_AddDesc_rpy_5fpose_2eproto();
  friend void protobuf_AssignDesc_rpy_5fpose_2eproto();
  friend void protobuf_ShutdownFile_rpy_5fpose_2eproto();

  void InitAsDefaultInstance();
  static RPYPose* default_instance_;
};
// ===================================================================


// ===================================================================

// RPYPose

// required double x = 1;
inline bool RPYPose::has_x() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void RPYPose::set_has_x() {
  _has_bits_[0] |= 0x00000001u;
}
inline void RPYPose::clear_has_x() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void RPYPose::clear_x() {
  x_ = 0;
  clear_has_x();
}
inline double RPYPose::x() const {
  // @@protoc_insertion_point(field_get:atl_msgs.msgs.RPYPose.x)
  return x_;
}
inline void RPYPose::set_x(double value) {
  set_has_x();
  x_ = value;
  // @@protoc_insertion_point(field_set:atl_msgs.msgs.RPYPose.x)
}

// required double y = 2;
inline bool RPYPose::has_y() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void RPYPose::set_has_y() {
  _has_bits_[0] |= 0x00000002u;
}
inline void RPYPose::clear_has_y() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void RPYPose::clear_y() {
  y_ = 0;
  clear_has_y();
}
inline double RPYPose::y() const {
  // @@protoc_insertion_point(field_get:atl_msgs.msgs.RPYPose.y)
  return y_;
}
inline void RPYPose::set_y(double value) {
  set_has_y();
  y_ = value;
  // @@protoc_insertion_point(field_set:atl_msgs.msgs.RPYPose.y)
}

// required double z = 3;
inline bool RPYPose::has_z() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void RPYPose::set_has_z() {
  _has_bits_[0] |= 0x00000004u;
}
inline void RPYPose::clear_has_z() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void RPYPose::clear_z() {
  z_ = 0;
  clear_has_z();
}
inline double RPYPose::z() const {
  // @@protoc_insertion_point(field_get:atl_msgs.msgs.RPYPose.z)
  return z_;
}
inline void RPYPose::set_z(double value) {
  set_has_z();
  z_ = value;
  // @@protoc_insertion_point(field_set:atl_msgs.msgs.RPYPose.z)
}

// required double roll = 4;
inline bool RPYPose::has_roll() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void RPYPose::set_has_roll() {
  _has_bits_[0] |= 0x00000008u;
}
inline void RPYPose::clear_has_roll() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void RPYPose::clear_roll() {
  roll_ = 0;
  clear_has_roll();
}
inline double RPYPose::roll() const {
  // @@protoc_insertion_point(field_get:atl_msgs.msgs.RPYPose.roll)
  return roll_;
}
inline void RPYPose::set_roll(double value) {
  set_has_roll();
  roll_ = value;
  // @@protoc_insertion_point(field_set:atl_msgs.msgs.RPYPose.roll)
}

// required double pitch = 5;
inline bool RPYPose::has_pitch() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void RPYPose::set_has_pitch() {
  _has_bits_[0] |= 0x00000010u;
}
inline void RPYPose::clear_has_pitch() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void RPYPose::clear_pitch() {
  pitch_ = 0;
  clear_has_pitch();
}
inline double RPYPose::pitch() const {
  // @@protoc_insertion_point(field_get:atl_msgs.msgs.RPYPose.pitch)
  return pitch_;
}
inline void RPYPose::set_pitch(double value) {
  set_has_pitch();
  pitch_ = value;
  // @@protoc_insertion_point(field_set:atl_msgs.msgs.RPYPose.pitch)
}

// required double yaw = 6;
inline bool RPYPose::has_yaw() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void RPYPose::set_has_yaw() {
  _has_bits_[0] |= 0x00000020u;
}
inline void RPYPose::clear_has_yaw() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void RPYPose::clear_yaw() {
  yaw_ = 0;
  clear_has_yaw();
}
inline double RPYPose::yaw() const {
  // @@protoc_insertion_point(field_get:atl_msgs.msgs.RPYPose.yaw)
  return yaw_;
}
inline void RPYPose::set_yaw(double value) {
  set_has_yaw();
  yaw_ = value;
  // @@protoc_insertion_point(field_set:atl_msgs.msgs.RPYPose.yaw)
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace msgs
}  // namespace atl_msgs

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_rpy_5fpose_2eproto__INCLUDED
