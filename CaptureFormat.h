// CaptureFilter.h

#ifndef _PPBOX_CAPTURE_RTM_FILTER_H_
#define _PPBOX_CAPTURE_RTM_FILTER_H_

#include <ppbox/demux/base/DemuxBase.h>

#include <boost/asio/buffer.hpp>

namespace ppbox
{
    namespace capture
    {

        struct CaptureBuffer
        {
            CaptureBuffer()
                : data(NULL)
                , len(0)
            {
            }

            boost::uint8_t const * data;
            boost::uint32_t len;
        };

        typedef ppbox::demux::StreamInfo CaptureStream;

        struct CaptureSample
        {
            CaptureSample()
                : itrack(boost::uint32_t(-1))
                , flags(0)
                , dts(0)
                , cts_delta(0)
                , duration(0)
                , size(0)
                , cbuf(0)
                , context(NULL)
            {
            }

            boost::uint32_t itrack;
            boost::uint32_t flags;
            boost::uint64_t dts;
            boost::uint32_t cts_delta;
            boost::uint32_t duration;
            boost::uint32_t size;
            boost::uint32_t cbuf;
            void const * context;

            boost::uint32_t real_size() const
            {
                return sizeof(CaptureSample) + sizeof(CaptureBuffer) * (cbuf);
            }
        };

        struct CaptureConfigData
        {
            CaptureConfigData()
                : stream_count(0)
                , get_sample_buffers(NULL)
                , free_sample(NULL)
            {
            }

            boost::uint32_t stream_count;
            bool (*get_sample_buffers)(void const *, CaptureBuffer *);
            bool (*free_sample)(void const *);
        };

    } // namespace mux
} // namespace ppbox

namespace boost
{
    namespace asio
    {
        inline boost::asio::const_buffers_1 buffer(ppbox::capture::CaptureBuffer const & buf)
        {
            return buffer(buf.data, buf.len);
        }
    }
}

#endif // _PPBOX_CAPTURE_RTM_FILTER_H_
