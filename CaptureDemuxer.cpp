// CaptureDemuxer.cpp

#include "ppbox/capture/Common.h"
#include "ppbox/capture/CaptureDemuxer.h"
#include "ppbox/capture/CaptureFilter.h"
#include "ppbox/capture/CaptureSource.h"

#include <ppbox/demux/packet/filter/SortFilter.h>
#include <ppbox/demux/basic/flv/FlvStream.h>
using namespace ppbox::demux;

#include <ppbox/avformat/flv/FlvTagType.h>
using namespace ppbox::avformat;

#include <util/buffers/BuffersCopy.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.capture.CaptureDemuxer", framework::logger::Debug);

namespace ppbox
{
    namespace capture
    {

        CaptureDemuxer::CaptureDemuxer(
            boost::asio::io_service & io_svc, 
            ppbox::data::PacketMedia & media)
            : ppbox::demux::PacketDemuxer(io_svc, media)
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
                static_cast<CaptureSource &>(source_->source()).free_sample(
                    (void *)(intptr_t)sample.memory->offset, ec);
            }
            return PacketDemuxer::free_sample(sample, ec);
        }

        bool CaptureDemuxer::check_open(
            boost::system::error_code & ec)
        {
            assert(filter_ == NULL);
            bool result = static_cast<CaptureSource &>(source_->source()).get_streams(stream_infos_, ec);
            if (result) {
                filter_ = new CaptureFilter;
                add_filter(filter_);
                add_filter(new SortFilter(stream_infos_.size()));
            }
            return result;
        }

    } // namespace capture
} // namespace ppbox
