#include "message_handler.hpp"

MessageHandler::MessageHandler( ip_t sender )
    : sender_( sender )
{
}

void MessageHandler::executeCommand( const PInnerContainer& container )
{
  if ( container.IsInitialized() )
  {
    if ( container.type() == SEND_CMD )
    {
      if ( container.has_message() )
      {
        PSendCommand nested_cont;
        nested_cont.ParseFromString( container.message() );
        if ( container.has_receiver() )
        {
          server_.sendMessageTo( nested_cont.cont_to_send(), container.receiver() );
        }
        else
        {
          server_.broadcastMessage( nested_cont.cont_to_send() );
        }
      }
      else
      {
        server_.broadcastPing();
      }
    }
  }
  else
  {
    Logger::error( "Cannot parse local message:" );
    Logger::error( container.InitializationErrorString() );
  }
}

void MessageHandler::handleReceivedMessage( const PInnerContainer& container )
{
  Logger::info("Signed message from "+sender_.to_string() +" received: "+ container.DebugString());
}
