
#pragma once

#include <ace/Svc_Handler.h>
#include <ace/Reactor.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <string>
#include "GadgetronSlotContainer.h"
#include "GadgetMessageInterface.h"
#include "GadgetronConnector.h"
#include "gadgettools_export.h"
#include "GadgetMRIHeaders.h"

#define GADGETRON_TIMEOUT_PERIOD 1.0

namespace Gadgetron
{

template<typename JobType> class GadgetCloudController;
template<typename JobType> class GadgetronCloudConnector;

template<typename JobType> 
class CloudWriterTask : public WriterTask
{

public:
    typedef WriterTask inherited;

    CloudWriterTask(ACE_SOCK_Stream* socket)
    : inherited(socket), cloud_connector_(NULL)
    {
    }

    virtual ~CloudWriterTask()
    {
    }

    void set_cloud_connector(GadgetronCloudConnector<JobType>* connector)
    {
        cloud_connector_ = connector;
    }

    virtual int svc(void)
    {
        ACE_Message_Block* mb = 0;
        while (this->getq (mb) != -1)
        {
            int retval = this->svcImpl(mb);

            if ( retval == 2 )
            {
                GADGET_DEBUG1("CloudWriterTask quit\n");
                return 0;
            }

            if ( retval == -1 )
            {
                GADGET_DEBUG1("CloudWriterTask svcImpl failed ... \n");
                ACE_OS::sleep(ACE_Time_Value(GADGETRON_TIMEOUT_PERIOD));
                return -1;
            }
        }

        return 0;
    }

    virtual int svcImpl(ACE_Message_Block* mb)
    {
        ACE_Time_Value nowait (ACE_OS::gettimeofday ());

        //Send a package if we have one
        GadgetContainerMessage<GadgetMessageIdentifier>* mid =
                AsContainerMessage<GadgetMessageIdentifier>(mb);

        if (!mid)
        {
            ACE_DEBUG ((LM_ERROR, ACE_TEXT ("Invalid message on output queue\n")));
            mb->release();
            return -1;
        }

        //Is this a shutdown message?
        if (mid->getObjectPtr()->id == GADGET_MESSAGE_CLOSE)
        {
            socket_->send_n(mid->getObjectPtr(),sizeof(GadgetMessageIdentifier));
            GADGET_DEBUG1("CloudWriterTask done\n");
            return 2;
        }

        GadgetMessageWriter* w = writers_.find(mid->getObjectPtr()->id);

        if (!w)
        {
            ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) Unrecognized Message ID received: %d\n"),mid->getObjectPtr()->id));
            mb->release();
            return -1;
        }

        if (w->write(socket_,mb->cont()) < 0)
        {
            ACE_DEBUG ( (LM_DEBUG, ACE_TEXT ("(%P|%t) Failed to write message to Gadgetron\n")) );

            // notice the controller
            GadgetContainerMessage<int>* m1 = 
                dynamic_cast< GadgetContainerMessage<int>* >(mb->cont());

            if ( m1 )
            {
                int jobID = *(m1->getObjectPtr());
                cloud_connector_->setJobTobeCompletedAndNoticeController(jobID);
            }
            else
            {
                cloud_connector_->setJobTobeCompletedAndNoticeController();
            }

            mb->release ();
            return -1;
        }

        mb->release();

        GADGET_DEBUG1("--> CloudWriterTask, write msg through socket done ... \n");

        return 0;
    }

protected:
    GadgetronCloudConnector<JobType>* cloud_connector_;
};

template<typename JobType> 
class CloudReaderTask : public ACE_Task<ACE_MT_SYNCH>
{

public:
    typedef ACE_Task<ACE_MT_SYNCH> inherited;

    CloudReaderTask(ACE_SOCK_Stream* socket) : inherited(), socket_(socket), cloud_connector_(NULL)
    {
    }

    virtual ~CloudReaderTask()
    {
        readers_.clear();
    }

    virtual int init(void)
    {
        ACE_TRACE(( ACE_TEXT("CloudReaderTask::init") ));
        return 0;
    }

    virtual int open(void* = 0)
    {
        GADGET_DEBUG1("CloudReaderTask::open\n");
        return this->activate( THR_NEW_LWP | THR_JOINABLE, 1 );
    }

    void set_cloud_connector(GadgetronCloudConnector<JobType>* connector)
    {
        cloud_connector_ = connector;
    }

    int register_reader(unsigned long long slot, GadgetMessageReader* reader)
    {
        return readers_.insert(slot,reader);
    }

    virtual int close(unsigned long flags)
    {
        GADGET_DEBUG1("CloudReaderTask::close\n");
        int rval = 0;
        if (flags == 1) {
            /*
            ACE_Message_Block *hangup = new ACE_Message_Block();
            hangup->msg_type( ACE_Message_Block::MB_HANGUP );
            if (this->putq(hangup) == -1) {
                hangup->release();
                ACE_ERROR_RETURN( (LM_ERROR,
                        ACE_TEXT("%p\n"),
                        ACE_TEXT("WriterTask::close, putq")),
                        -1);
            }
            */
            rval = this->wait();
        }
        return rval;
    }

    virtual int svc(void)
    {
        ssize_t recv_count = 0;
        GadgetMessageIdentifier mid;

        while (1)
        {
            if ((recv_count = cloud_connector_->peer().recv_n(&mid, sizeof(GadgetMessageIdentifier))) <= 0)
            {
                ACE_DEBUG( (LM_ERROR, ACE_TEXT("%P, %l, CloudReaderTask, failed to read message identifier\n")) );
                ACE_OS::sleep(ACE_Time_Value(GADGETRON_TIMEOUT_PERIOD));
                cloud_connector_->set_status(false);
                cloud_connector_->setJobTobeCompletedAndNoticeController();
                return -1;
            }

            //Is this a shutdown message?
            if (mid.id == GADGET_MESSAGE_CLOSE)
            {
                ACE_DEBUG( (LM_INFO, ACE_TEXT("%P, %l, CloudReaderTask, Close Message received\n")) );
                return 0;
            }

            GadgetMessageReader* r = readers_.find(mid.id);
            if (r == 0)
            {
                ACE_DEBUG( (LM_ERROR, ACE_TEXT("%P, %l, CloudReaderTask, Unknown message id %d received\n"), mid.id) );
                cloud_connector_->set_status(false);
                cloud_connector_->setJobTobeCompletedAndNoticeController();
                return -1;
            }

            ACE_Message_Block* mb = r->read(&cloud_connector_->peer());

            if (!mb)
            {
                ACE_DEBUG( (LM_ERROR, ACE_TEXT("%P, %l, CloudReaderTask, Failed to read message\n")) );
                ACE_OS::sleep(ACE_Time_Value(GADGETRON_TIMEOUT_PERIOD));
                cloud_connector_->set_status(false);
                cloud_connector_->setJobTobeCompletedAndNoticeController();
                return -1;
            }
            else
            {
                if (cloud_connector_->process(mid.id, mb) < 0)
                {
                    ACE_DEBUG( (LM_ERROR, ACE_TEXT("%P, %l, ReaderTask, Failed to process message\n")) );
                    cloud_connector_->set_status(false);
                    cloud_connector_->setJobTobeCompletedAndNoticeController();
                    return -1;
                }
            }
        }

        GADGET_DEBUG1("CloudReaderTask, stop with return value 0 ... \n");
        return 0;
    }

protected:

    ACE_SOCK_Stream* socket_;
    GadgetronSlotContainer<GadgetMessageReader> readers_;
    GadgetronCloudConnector<JobType>* cloud_connector_;
};

template<typename JobType> 
class GadgetronCloudConnector
{
public:

    GadgetronCloudConnector();
    virtual ~GadgetronCloudConnector();

    int openImpl (std::string hostname, std::string port);
    int open (std::string hostname, std::string port);

    virtual int process(unsigned long long messageid, ACE_Message_Block* mb);

    void set_cloud_controller(GadgetCloudController<JobType>* controller);

    // if jobID==-1, all jobs for this node is set to be completed
    int setJobTobeCompletedAndNoticeController(int jobID=-1);

    virtual int putq  (  ACE_Message_Block * mb ,  ACE_Time_Value *  timeout = 0);

    virtual int register_reader(unsigned long long slot, GadgetMessageReader* reader);
    virtual int register_writer(unsigned long long slot, GadgetMessageWriter* writer);

    int close()
    {
        GADGET_DEBUG1("Into GadgetronCloudConnector:close() ... \n");
        GADGET_DEBUG1("Closing socket \n");
        peer().close();
        GADGET_DEBUG1("Socket closed \n");
        cloud_writer_task_.flush();
        cloud_reader_task_.close(0);
        cloud_writer_task_.close(0);
        return this->wait();
    }

    virtual int wait()
    {
        GADGET_DEBUG1("Into GadgetronCloudConnector:wait() ... \n");

        int retval;
        GADGET_DEBUG1("Waiting for cloud reader task:\n");
        retval = cloud_reader_task_.wait();
        GADGET_DEBUG1("Reader task done\n");

        ACE_TRACE(( ACE_TEXT("Waiting for cloud writer task:") ));
        retval = cloud_writer_task_.wait();
        ACE_TRACE(( ACE_TEXT("Writer task done:") ));

        return retval;
    }

    CloudWriterTask<JobType>& writer_task()
    {
        return cloud_writer_task_;
    }

    CloudReaderTask<JobType>& reader_task()
    {
        return cloud_reader_task_;
    }

    bool status()
     {
        bool ret_val;
        mtx_.acquire();
        ret_val = status_;
        mtx_.release();
        return ret_val;
    }

    void set_status(bool s)
    {
        mtx_.acquire();
        status_ = s;
        mtx_.release();
    }

    int send_gadgetron_configuration_file(std::string config_xml_name);
    int send_gadgetron_configuration_script(std::string config_xml_name);
    int send_gadgetron_parameters(std::string xml_string);

    ACE_SOCK_Stream& peer()
    {
        return peer_;
    }

    unsigned int nodeID_;

protected:

    ACE_Thread_Mutex mtx_;
    bool status_;

    std::string hostname_;
    std::string port_;

    GadgetCloudController<JobType>* cloud_controller_;
    CloudWriterTask<JobType> cloud_writer_task_;
    CloudReaderTask<JobType> cloud_reader_task_;

    ACE_SOCK_Stream peer_;
};

template<typename JobType> 
GadgetronCloudConnector<JobType>::GadgetronCloudConnector() : cloud_controller_(NULL), 
                                                            nodeID_(0), 
                                                            cloud_writer_task_(&this->peer()), 
                                                            cloud_reader_task_(&this->peer()), 
                                                            status_(false), 
                                                            mtx_("CLOUDCONNECTOR_MTX")
{
    GADGET_DEBUG1("Into GadgetronCloudConnector:GadgetronCloudConnector() ... \n");
}

template<typename JobType> 
GadgetronCloudConnector<JobType>::~GadgetronCloudConnector()
{
    GADGET_DEBUG1("Into GadgetronCloudConnector:~GadgetronCloudConnector() ... \n");
    cloud_writer_task_.msg_queue()->deactivate();
    cloud_reader_task_.msg_queue()->deactivate();
    this->wait();
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::openImpl(std::string hostname, std::string port)
{
    hostname_= hostname;
    port_ = port;

    ACE_INET_Addr server(port_.c_str(),hostname_.c_str());
    ACE_SOCK_Connector connector;

    if (connector.connect(this->peer(),server) == -1)
    {
        ACE_ERROR_RETURN(( LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("connect")), -1);
    }

    ACE_TCHAR peer_name[MAXHOSTNAMELENGTH];
    ACE_INET_Addr peer_addr;
    if (peer().get_remote_addr (peer_addr) == 0 && peer_addr.addr_to_string (peer_name, MAXHOSTNAMELENGTH) == 0)
    {
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%P|%t) Connection from %s\n"), peer_name));
    }

    return 0;
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::open(std::string hostname, std::string port)
{
    this->cloud_writer_task_.set_cloud_connector(this);
    this->cloud_reader_task_.set_cloud_connector(this);

    if ( this->openImpl(hostname, port) == 0 )
    {
        status_ = true;
        this->cloud_writer_task_.open();
        this->cloud_reader_task_.open();
    }
    else
    {
        status_ = false;
    }

    return 0;
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::process(unsigned long long messageid, ACE_Message_Block* mb)
{
    // insert message into the queue of cloud controller
    if ( cloud_controller_ == NULL )
    {
        ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) GadgetronCloudConnector, pointer of could controller is null ...\n")));
        return -1;
    }

    cloud_controller_->putq(mb);

    return 0;
}

template<typename JobType> 
void GadgetronCloudConnector<JobType>::set_cloud_controller(GadgetCloudController<JobType>* controller)
{
    cloud_controller_ = controller;
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::putq(ACE_Message_Block* mb ,  ACE_Time_Value* timeout)
{
    return cloud_writer_task_.putq(mb,timeout);
    /*int retval = cloud_writer_task_.svcImpl(mb);
    if ( retval != 0 )
    {
        ACE_Time_Value tv(GADGETRON_TIMEOUT_PERIOD);
        ACE_OS::sleep(tv);
    }
    return retval;*/
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::register_reader(unsigned long long slot, GadgetMessageReader* reader)
{
    return cloud_reader_task_.register_reader(slot, reader);
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::register_writer(unsigned long long slot, GadgetMessageWriter* writer)
{
    return cloud_writer_task_.register_writer(slot,writer);
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::setJobTobeCompletedAndNoticeController(int jobID)
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, mtx_, -1);

    ACE_DEBUG( (LM_INFO, ACE_TEXT("%P, %l, GadgetronCloudConnector, into setJobTobeCompletedAndNoticeController(...) ... \n")) );

    if ( cloud_controller_->setJobsTobeCompleted(nodeID_, jobID) < 0 )
    {
        ACE_DEBUG( (LM_ERROR, ACE_TEXT("%P, %l, GadgetronCloudConnector, cloud_controller_->setJobsTobeCompleted(%d, %d) failed ... \n"), nodeID_, jobID) );
        return -1;
    }

    // put a invalid jobID==-1 to the controller message queue to trick the check
    GadgetContainerMessage<int>* jobIDMsg = new GadgetContainerMessage<int>();
    *(jobIDMsg->getObjectPtr()) = -1;

    if (process(GADGET_MESSAGE_CLOUD_JOB, jobIDMsg) < 0)
    {
        ACE_DEBUG( (LM_ERROR, ACE_TEXT("%P, %l, GadgetronCloudConnector, Failed to put jobIDMsg==-1 into the controller message queue\n")) );
        return -1;
    }

    return 0;
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::send_gadgetron_configuration_file(std::string config_xml_name)
{
    GadgetMessageIdentifier id;
    id.id = GADGET_MESSAGE_CONFIG_FILE;

    GadgetMessageConfigurationFile ini;
    ACE_OS_String::strncpy(ini.configuration_file, config_xml_name.c_str(),1024);

    if (this->peer().send_n(&id, sizeof(GadgetMessageIdentifier)) != sizeof(GadgetMessageIdentifier))
    {
        ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) Unable to send GadgetMessageIdentifier\n")));
        return -1;
    }

    if (this->peer().send_n(&ini, sizeof(GadgetMessageConfigurationFile)) != sizeof(GadgetMessageConfigurationFile))
     {
        ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) Unable to send GadgetMessageConfigurationFile\n")));
        return -1;
    }

    return 0;
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::send_gadgetron_configuration_script(std::string config_xml)
{
    GadgetMessageIdentifier id;
    id.id = GADGET_MESSAGE_CONFIG_SCRIPT;

    GadgetMessageScript ini;
    ini.script_length = config_xml.size()+1;

    if (this->peer().send_n(&id, sizeof(GadgetMessageIdentifier)) != sizeof(GadgetMessageIdentifier))
    {
        ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) Unable to send GadgetMessageIdentifier\n")));
        return -1;
    }

    if (this->peer().send_n(&ini, sizeof(GadgetMessageScript)) != sizeof(GadgetMessageScript))
    {
        ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) Unable to send GadgetMessageScript\n")));
        return -1;
    }

    if (this->peer().send_n(config_xml.c_str(), ini.script_length) != ini.script_length)
    {
        ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) Unable to send parameter xml\n")));
        return -1;
    }

    return 0;
}

template<typename JobType> 
int GadgetronCloudConnector<JobType>::send_gadgetron_parameters(std::string xml_string)
{
    GadgetMessageIdentifier id;
    id.id = GADGET_MESSAGE_PARAMETER_SCRIPT;

    GadgetMessageScript conf;
    conf.script_length = xml_string.size()+1;
    if (this->peer().send_n(&id, sizeof(GadgetMessageIdentifier)) != sizeof(GadgetMessageIdentifier))
    {
        ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) Unable to send GadgetMessageIdentifier\n")));
        return -1;
    }

    if (this->peer().send_n(&conf, sizeof(GadgetMessageScript)) != sizeof(GadgetMessageScript))
    {
        ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) Unable to send GadgetMessageScript\n")));
        return -1;
    }

    if (this->peer().send_n(xml_string.c_str(), conf.script_length) != conf.script_length)
    {
        ACE_DEBUG ((LM_ERROR, ACE_TEXT ("(%P|%t) Unable to send parameter xml\n")));
        return -1;
    }

    return 0;
}

}
