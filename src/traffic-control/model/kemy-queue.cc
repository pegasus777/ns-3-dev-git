#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "kemy-queue.h"



KemyQueueDisc::KemyQueueDisc() :
    _whiskers(NULL),
    _memory(),
    _the_window(0)
    {
    	/* store whiskers */
    	_whiskers = new WhiskerTree( tree );

        q_ = new PacketQueue();			    // inner queue
        pq_ = q_;

    }

KemyQueueDisc::~KemyQueueDisc(){
    delete q_;
}

bool
KemyQueueDisc::DoEnqueue (Ptr<QueueDiscItem> p)
{
	if (summarystats) {
                updateStats(qib_?q_->GetInternalQueue(0)->GetNBytes():q_->GetInternalQueue(0)->GetNPackets()); 
	}

//    hdr_ip* iph = hdr_ip::access(p);
    Ipv4Header ip;
    p->PeekHeader (ip);
    if (GetQueueSize()+1 > qlim_){
        Drop (p);
        return;
    }
    double tickno = Simulator::Now ().GetSeconds ();
    const Whisker & current_whisker(_whiskers->use_whisker(_memory));
     _the_window = current_whisker.window(_the_window);
    if( GetQueueSize() >= _the_window)
    {
        _memory.packet_drop(tickno, q_->length());
        Drop (p);
    }
    else
    {
        _memory.packet_receive(tickno, q_->length());
        q_->DoEnqueue(p);
    }
}
Ptr<QueueDiscItem>
KemyQueueDisc::DoDequeue (void)
{
	if (summarystats) {
                updateStats(qib_?q_->byteLength():q_->length());
	}
    return q_->DoDequeue();
}

uint32_t
KemyQueueDisc::GetQueueSize (void)
{
  NS_LOG_FUNCTION (this);
  if (GetMode () == Queue::QUEUE_MODE_BYTES)
    {
      return GetInternalQueue (0)->GetNBytes ();
    }
  else if (GetMode () == Queue::QUEUE_MODE_PACKETS)
    {
      return GetInternalQueue (0)->GetNPackets ();
    }
  else
    {
      NS_ABORT_MSG ("Unknown PIE mode.");
    }
}

void KemyQueueDisc::updateStats(int queuesize)
{
        double now = Simulator::Now ().GetSeconds ();
        double newtime = now - total_time_;
        if (newtime > 0.0) {
                double oldave = true_ave_;
                double oldtime = total_time_;
                double newtime = now - total_time_;
                true_ave_ = (oldtime * oldave + newtime * queuesize) /now;
                total_time_ = now;
        }
}
