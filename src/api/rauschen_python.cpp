#include "api/rauschen.h"
#include <boost/python.hpp>
#include <boost/python/args.hpp>

namespace bp = boost::python;

BOOST_PYTHON_MODULE(rauschen_python)
{
  /*
  bp::class_<CLogProxy, boost::noncopyable>("CLog")
    .def(bp::init<const bp::list&, log_t>())
    .def(bp::init<const std::string&, log_t>())
    .def("Write", &CLogProxy::Write)
    .def("Fill", &CLogProxy::Fill)
    .def("RecoverTail", &CLogProxy::RecoverTail)
    .def("SimpleScan", &CLogProxy::SimpleScan)
    .def("Scan", &CLogProxy::Scan)
    .def("Probe", &CLogProxy::Probe)
    .def("Trim", &CLogProxy::Trim)
    .def("SetTimeout", &CLogProxy::SetTimeout)
    .def("GlobalTrim", &CLogProxy::GlobalTrim)
    .def("DoesLogProvideSequencer", &CLogProxy::DoesLogProvideSequencer)
    .def("GetReadTimestamp", &CLogProxy::GetReadTimestamp)
    .def("GetWriteTimestamp", &CLogProxy::GetWriteTimestamp)
    .def("IncrementSequencerEpoch", &CLogProxy::IncrementSequencerEpoch);*/

  bp::enum_<rauschen_status>("rauschen_status")
    .value("OK", RAUSCHEN_STATUS_OK)
    .value("ERROR", RAUSCHEN_STATUS_ERR);

  bp::def("rauschen_send_message", rauschen_send_message, ((bp::arg("message") = "", bp::arg("message_type") = "", bp::arg("receiver") = "")));
}
