#ifndef PLANHDR_HPP
#define PLANHDR_HPP

#include <ihandler.hpp>

#include "eventid.h"

class PlannerHandler : public IHandler
{
   public:
     Boolean
        dispatchHandlerEvent( IEvent& evt);

   protected:
     virtual Boolean plannerMessage( IEvent& evt);
};
#endif
