// CaptureModule.h

#ifndef _JUST_CAPTURE_capture_MODULE_H_
#define _JUST_CAPTURE_capture_MODULE_H_

#include <framework/string/Url.h>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace just
{
    namespace capture
    {

        class CaptureSource;

        class CaptureModule 
            : public just::common::CommonModuleBase<CaptureModule>
        {
        public:
            CaptureModule(
                util::daemon::Daemon & daemon);

            virtual ~CaptureModule();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();

        public:
            CaptureSource * create(
                framework::string::Url const & play_link, 
                boost::system::error_code & ec);

            bool destroy(
                CaptureSource * capture, 
                boost::system::error_code & ec);

        private:
            struct CaptureInfo;

        private:
            std::vector<CaptureInfo *> captures_;
            boost::mutex mutex_;
            boost::condition_variable cond_;
        };

    } // namespace capture
} // namespace just

#endif // _JUST_CAPTURE_capture_MODULE_H_