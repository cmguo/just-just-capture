// CaptureDemuxer.cpp

#include "just/capture/Common.h"
#include "just/capture/CaptureDemuxer.h"
#include "just/capture/CaptureFilter.h"
#include "just/capture/CaptureSource.h"

using namespace just::demux;

#include <util/buffers/BuffersCopy.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.capture.CaptureDemuxer", framework::logger::Debug);

namespace just
{
    namespace capture
    {

        CaptureDemuxer::CaptureDemuxer(
            boost::asio::io_service & io_svc, 
            just::data::PacketMedia & media)
            : just::demux::PacketDemuxer(io_svc, media)
            , filter_(NULL)
        {
        }

        CaptureDemuxer::~CaptureDemuxer()
        {
            if (filter_) {
                delete filter_;
            }
        }

        bool CaptureDemuxer::free_sample(
            Sample & sample, 
            boost::system::error_code & ec)
        {
            if (sample.memory) {
                static_cast<CaptureSource &>(source_->stream_source()).free_sample(
                    *(CaptureSample const *)(intptr_t)sample.memory->offset, ec);
            }
            return PacketDemuxer::free_sample(sample, ec);
        }

        bool CaptureDemuxer::check_open(
            boost::system::error_code & ec)
        {
            assert(filter_ == NULL);
            bool result = static_cast<CaptureSource &>(source_->stream_source()).get_streams(stream_infos_, ec);
            if (result) {
                filter_ = new CaptureFilter(static_cast<CaptureSource &>(source().stream_source()));
                add_filter(filter_);
            }
            return result;
        }

    } // namespace capture
} // namespace just
