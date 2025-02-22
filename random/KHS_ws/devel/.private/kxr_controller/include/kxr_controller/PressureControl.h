// Generated by gencpp from file kxr_controller/PressureControl.msg
// DO NOT EDIT!


#ifndef KXR_CONTROLLER_MESSAGE_PRESSURECONTROL_H
#define KXR_CONTROLLER_MESSAGE_PRESSURECONTROL_H


#include <string>
#include <vector>
#include <memory>

#include <ros/types.h>
#include <ros/serialization.h>
#include <ros/builtin_message_traits.h>
#include <ros/message_operations.h>


namespace kxr_controller
{
template <class ContainerAllocator>
struct PressureControl_
{
  typedef PressureControl_<ContainerAllocator> Type;

  PressureControl_()
    : board_idx(0)
    , start_pressure(0.0)
    , stop_pressure(0.0)
    , release(false)  {
    }
  PressureControl_(const ContainerAllocator& _alloc)
    : board_idx(0)
    , start_pressure(0.0)
    , stop_pressure(0.0)
    , release(false)  {
  (void)_alloc;
    }



   typedef uint16_t _board_idx_type;
  _board_idx_type board_idx;

   typedef float _start_pressure_type;
  _start_pressure_type start_pressure;

   typedef float _stop_pressure_type;
  _stop_pressure_type stop_pressure;

   typedef uint8_t _release_type;
  _release_type release;





  typedef boost::shared_ptr< ::kxr_controller::PressureControl_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::kxr_controller::PressureControl_<ContainerAllocator> const> ConstPtr;

}; // struct PressureControl_

typedef ::kxr_controller::PressureControl_<std::allocator<void> > PressureControl;

typedef boost::shared_ptr< ::kxr_controller::PressureControl > PressureControlPtr;
typedef boost::shared_ptr< ::kxr_controller::PressureControl const> PressureControlConstPtr;

// constants requiring out of line definition



template<typename ContainerAllocator>
std::ostream& operator<<(std::ostream& s, const ::kxr_controller::PressureControl_<ContainerAllocator> & v)
{
ros::message_operations::Printer< ::kxr_controller::PressureControl_<ContainerAllocator> >::stream(s, "", v);
return s;
}


template<typename ContainerAllocator1, typename ContainerAllocator2>
bool operator==(const ::kxr_controller::PressureControl_<ContainerAllocator1> & lhs, const ::kxr_controller::PressureControl_<ContainerAllocator2> & rhs)
{
  return lhs.board_idx == rhs.board_idx &&
    lhs.start_pressure == rhs.start_pressure &&
    lhs.stop_pressure == rhs.stop_pressure &&
    lhs.release == rhs.release;
}

template<typename ContainerAllocator1, typename ContainerAllocator2>
bool operator!=(const ::kxr_controller::PressureControl_<ContainerAllocator1> & lhs, const ::kxr_controller::PressureControl_<ContainerAllocator2> & rhs)
{
  return !(lhs == rhs);
}


} // namespace kxr_controller

namespace ros
{
namespace message_traits
{





template <class ContainerAllocator>
struct IsMessage< ::kxr_controller::PressureControl_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::kxr_controller::PressureControl_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct IsFixedSize< ::kxr_controller::PressureControl_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsFixedSize< ::kxr_controller::PressureControl_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct HasHeader< ::kxr_controller::PressureControl_<ContainerAllocator> >
  : FalseType
  { };

template <class ContainerAllocator>
struct HasHeader< ::kxr_controller::PressureControl_<ContainerAllocator> const>
  : FalseType
  { };


template<class ContainerAllocator>
struct MD5Sum< ::kxr_controller::PressureControl_<ContainerAllocator> >
{
  static const char* value()
  {
    return "61bcf3809e7f0e409628cc484675109d";
  }

  static const char* value(const ::kxr_controller::PressureControl_<ContainerAllocator>&) { return value(); }
  static const uint64_t static_value1 = 0x61bcf3809e7f0e40ULL;
  static const uint64_t static_value2 = 0x9628cc484675109dULL;
};

template<class ContainerAllocator>
struct DataType< ::kxr_controller::PressureControl_<ContainerAllocator> >
{
  static const char* value()
  {
    return "kxr_controller/PressureControl";
  }

  static const char* value(const ::kxr_controller::PressureControl_<ContainerAllocator>&) { return value(); }
};

template<class ContainerAllocator>
struct Definition< ::kxr_controller::PressureControl_<ContainerAllocator> >
{
  static const char* value()
  {
    return "uint16 board_idx  # Do not use board id? (Currently, bus-connected air board is not supported)\n"
"float32 start_pressure  # Threshold [kPa] to start pump (Currently, pressurization is not supported)\n"
"float32 stop_pressure  # Threshold [kPa] to stop pump (Currently, pressurization is not supported)\n"
"bool release  # Set true to release air by opening valve.\n"
;
  }

  static const char* value(const ::kxr_controller::PressureControl_<ContainerAllocator>&) { return value(); }
};

} // namespace message_traits
} // namespace ros

namespace ros
{
namespace serialization
{

  template<class ContainerAllocator> struct Serializer< ::kxr_controller::PressureControl_<ContainerAllocator> >
  {
    template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
    {
      stream.next(m.board_idx);
      stream.next(m.start_pressure);
      stream.next(m.stop_pressure);
      stream.next(m.release);
    }

    ROS_DECLARE_ALLINONE_SERIALIZER
  }; // struct PressureControl_

} // namespace serialization
} // namespace ros

namespace ros
{
namespace message_operations
{

template<class ContainerAllocator>
struct Printer< ::kxr_controller::PressureControl_<ContainerAllocator> >
{
  template<typename Stream> static void stream(Stream& s, const std::string& indent, const ::kxr_controller::PressureControl_<ContainerAllocator>& v)
  {
    s << indent << "board_idx: ";
    Printer<uint16_t>::stream(s, indent + "  ", v.board_idx);
    s << indent << "start_pressure: ";
    Printer<float>::stream(s, indent + "  ", v.start_pressure);
    s << indent << "stop_pressure: ";
    Printer<float>::stream(s, indent + "  ", v.stop_pressure);
    s << indent << "release: ";
    Printer<uint8_t>::stream(s, indent + "  ", v.release);
  }
};

} // namespace message_operations
} // namespace ros

#endif // KXR_CONTROLLER_MESSAGE_PRESSURECONTROL_H
