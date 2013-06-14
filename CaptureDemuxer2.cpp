// CaptureDemuxer2.cpp

#include "ppbox/capture/Common.h"
#include "ppbox/capture/CaptureDemuxer2.h"
#include "ppbox/capture/CaptureFilter2.h"
#include "ppbox/capture/CaptureSource2.h"

#include <ppbox/demux/basic/flv/FlvStream.h>
using namespace ppbox::demux;

#include <ppbox/avformat/flv/FlvTagType.h>
using namespace ppbox::avformat;

#include <util/buffers/BuffersCopy.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.capture.CaptureDemuxer2", framework::logger::Debug);

namespace ppbox
{
    namespace capture
    {

        CaptureDemuxer2::CaptureDemuxer2(
            boost::asio::io_service & io_svc, 
            ppbox::data::PacketMedia & media)
            : ppbox::demux::PacketDemuxer(io_svc, media)
            , filter_(NULL)
        {
        }

        CaptureDemuxer2::~CaptureDemuxer2()
        {
            if (filter_) {
                delete filter_;
            }
        }

        bool CaptureDemuxer2::free_sample(
            Sample & sample, 
            boost::system::error_code & ec)
        {
            if (sample.memory) {
                static_cast<CaptureSource2 &>(source_->source()).free_sample(
                    *(CaptureSample const *)(intptr_t)sample.memory->offset, ec);
            }
            return PacketDemuxer::free_sample(sample, ec);
        }

        bool CaptureDemuxer2::check_open(
            boost::system::error_code & ec)
        {
            assert(filter_ == NULL);
            bool result = static_cast<CaptureSource2 &>(source_->source()).get_streams(stream_infos_, ec);
            if (result) {
                filter_ = new CaptureFilter2(static_cast<CaptureSource2 &>(source().source()));
                add_filter(filter_);
            }
            return result;
        }

    } // namespace capture
} // namespace ppbox
