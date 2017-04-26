#ifndef KEMY_QUEUE_HH
#define KEMY_QUEUE_HH



#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "kemy-queue.h"



namespace ns3 {

class TraceContainer;

class KemyQueueDisc : public QueueDisc
{
public:
  KemyQueueDisc ();
  virtual ~KemyQueueDisc ();


    protected:
        WhiskerTree*  _whiskers;
        Memory _memory;
        uint32_t _the_window;
        uint32_t summarystats;

        uint32_t qib_;          /* bool: queue measured in bytes? */
        Ptr<QueueDiscItem> q_; 
        
    private:
        virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
        virtual Ptr<QueueDiscItem> DoDequeue (void);
        virtual Ptr<const QueueDiscItem> DoPeek (void) const;
        virtual bool CheckConfig (void);
};
}
#endif
