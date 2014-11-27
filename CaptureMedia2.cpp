// CaptureMedia2.cpp

#include "just/capture/Common.h"
#include "just/capture/CaptureMedia2.h"
#include "just/capture/CaptureModule.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/string/Algorithm.h>

#include <boost/bind.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.capture.CaptureMedia2", framework::logger::Debug);

namespace just
{
    namespace capture
    {

        CaptureMedia2::CaptureMedia2(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PacketMedia(io_svc, url)
            , source_(NULL)
        {
            boost::system::error_code ec;
            PacketMedia::get_basic_info(info_, ec);
            info_.type = info_.live;
            info_.format_type = "capture2";
        }

        CaptureMedia2::~CaptureMedia2()
        {
        }

        void CaptureMedia2::async_open(
            MediaBase::response_type const & resp)
        {
            boost::system::error_code ec;
            source_ = new CaptureSource2(get_io_service());
            source_->async_open(url_, resp);
        }

        void CaptureMedia2::cancel(
            boost::system::error_code & ec)
        {
            source_->cancel(ec);
        }

        void CaptureMedia2::close(
            boost::system::error_code & ec)
        {
            source_->close(ec);
            delete source_;
            source_ = NULL;
        }

        bool CaptureMedia2::get_basic_info(
            just::data::MediaBasicInfo & info,
            boost::system::error_code & ec) const
        {
            info = info_;
            ec.clear();
            return true;
        }

        bool CaptureMedia2::get_info(
            just::data::MediaInfo & info,
            boost::system::error_code & ec) const
        {
            info = info_;
            ec.clear();
            return PacketMedia::get_info(info, ec);
        }

        bool CaptureMedia2::get_packet_feature(
            just::data::PacketFeature & feature,
            boost::system::error_code & ec) const
        {
            ec.clear();
            feature = source_->feature();
            return true;
        }

        util::stream::Source & CaptureMedia2::source()
        {
            return *source_;
        }

    } // namespace capture
} // namespace just
