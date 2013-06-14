// CaptureFilter.cpp

#include "ppbox/capture/Common.h"
#include "ppbox/capture/CaptureFilter.h"
#include "ppbox/capture/CaptureSource.h"

#include <ppbox/demux/base/DemuxError.h>
using namespace ppbox::demux;

#include <ppbox/data/base/MemoryLock.h>

#include <util/buffers/BuffersCopy.h>

namespace ppbox
{
    namespace capture
    {

        CaptureFilter::CaptureFilter(
            CaptureSource & source)
            : source_(source)
        {
        }

        CaptureFilter::~CaptureFilter()
        {
        }

        bool CaptureFilter::get_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (!Filter::get_sample(sample, ec))
                return false;

            CaptureSample const & header = 
                *(boost::asio::buffer_cast<CaptureSample const *>(sample.data.front()));

            assert(sample.size == sizeof(header));
            assert(boost::asio::buffer_size(sample.data.front()) == sizeof(header));
            sample.data.clear();

            sample.itrack = header.itrack;
            sample.flags = header.flags;
            sample.dts = header.dts;
            sample.cts_delta = header.cts_delta;
            sample.duration = header.duration;
            sample.memory->offset = (intptr_t)&header;

            if (header.buffer == NULL) {
                if (buffers_.size() < header.size) {
                    buffers_.resize(header.size);
                }
                source_.get_sample_buffers(header, buffers_, ec);
                boost::uint32_t size = 0;
                for (boost::uint32_t i = 0; i < header.size; ++i) {
                    sample.data.push_back(boost::asio::buffer(buffers_[i]));
                    size += buffers_[i].len;
                }
                sample.size = size;
            } else {
                sample.size = header.size;
                sample.data.clear();
                sample.data.push_back(boost::asio::buffer(header.buffer, header.size));
            }

            return true;
        }

        bool CaptureFilter::get_next_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (!Filter::get_next_sample(sample, ec))
                return false;

            parse_for_time(sample, ec);

            return true;
        }

        bool CaptureFilter::get_last_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (!Filter::get_last_sample(sample, ec))
                return false;

            parse_for_time(sample, ec);

            return true;
        }

        void CaptureFilter::parse_for_time(
            Sample & sample,
            boost::system::error_code & ec)
        {
            CaptureSample const & header = 
                *(boost::asio::buffer_cast<CaptureSample const *>(sample.data.front()));

            sample.itrack = header.itrack;
            sample.dts = header.dts;
        }

    } // namespace mux
} // namespace ppbox
