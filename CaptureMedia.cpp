// CaptureMedia.cpp

#include "ppbox/capture/Common.h"
#include "ppbox/capture/CaptureMedia.h"
#include "ppbox/capture/CaptureModule.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/string/Algorithm.h>

#include <boost/bind.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.capture.CaptureMedia", framework::logger::Debug);

namespace ppbox
{
    namespace capture
    {

        CaptureMedia::CaptureMedia(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PacketMedia(io_svc, url)
            , mod_(util::daemon::use_module<CaptureModule>(io_svc))
            , source_(NULL)
        {
            boost::system::error_code ec;
            PacketMedia::get_basic_info(info_, ec);
            info_.type = info_.live;
            info_.format = "capture";
        }

        CaptureMedia::~CaptureMedia()
        {
        }

        void CaptureMedia::async_open(
            MediaBase::response_type const & resp)
        {
            boost::system::error_code ec;
            source_ = mod_.create(url_, ec);
            if (source_ == NULL) {
                get_io_service().post(
                    boost::bind(resp, ec));
            } else {
                source_->async_open(url_, resp);
            }
        }

        void CaptureMedia::cancel(
            boost::system::error_code & ec)
        {
            source_->cancel(ec);
        }

        void CaptureMedia::close(
            boost::system::error_code & ec)
        {
            source_->close(ec);
            mod_.destroy(source_, ec);
            source_ = NULL;
        }

        bool CaptureMedia::get_basic_info(
            ppbox::data::MediaBasicInfo & info,
            boost::system::error_code & ec) const
        {
            info = info_;
            ec.clear();
            return true;
        }

        bool CaptureMedia::get_info(
            ppbox::data::MediaInfo & info,
            boost::system::error_code & ec) const
        {
            info = info_;
            if (!(source_->config().flags & CaptureConfigData::f_stream_ordered)) {
                info.flags |= ppbox::data::PacketMediaFlags::f_non_ordered;
            }
            ec.clear();
            return PacketMedia::get_info(info, ec);
        }

        bool CaptureMedia::get_packet_feature(
            ppbox::data::PacketFeature & feature,
            boost::system::error_code & ec) const
        {
            feature.piece_size = sizeof(CaptureSample);
            feature.packet_max_size = sizeof(CaptureSample);
            feature.buffer_size = 1024 * 20; // 20K
            feature.prepare_size = feature.packet_max_size * 2;
            ec.clear();
            return true;
        }

        util::stream::Source & CaptureMedia::source()
        {
            return *source_;
        }

    } // namespace capture
} // namespace ppbox
