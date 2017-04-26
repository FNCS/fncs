program main
  implicit none
#include "fncs.fh"
  integer ( 8 ) :: current_time
  integer ( 8 ) :: time_requested
  integer ( 8 ) :: event_size
  integer ( 8 ) :: event_index
  integer :: fed_id
  integer :: fed_size
  integer :: major
  integer :: minor
  integer :: patch
! character strings are copied from FNCS, so we need something big
! enough to hold names, event names, event values, etc.
  character ( len=1024 ) :: config
  character ( len=1024 ) :: my_name
  character ( len=1024 ) :: event_name
  character ( len=1024 ) :: event_value
  character :: TAB
  TAB = char(9)

  config = "name = fcoverage"//NEW_LINE('A')//&
"time_delta = 1s"//NEW_LINE('A')//&
"values"//NEW_LINE('A')//&
"    fcoverage/key"//NEW_LINE('A')//&
"    player/key"//NEW_LINE('A')//&
"    pycoverage/key"//NEW_LINE('A')//&
"    pycoverage/key"//NEW_LINE('A')//&
"    key_anon"//NEW_LINE('A')//&
"        topic = global/key_anon"//NEW_LINE('A')//&
"        list = true"//NEW_LINE('A')

!  write (*,'(A)') trim(config)

  call fncs_get_version ( major, minor, patch )
  write (*,'(A,I1,A,I1,A,I1)') 'fcoverage test running FNCS version ',major,'.',minor,'.',patch

  call fncs_initialize_config ( config )
  if ( .NOT. fncs_is_initialized ( ) ) then
    write (*,'(A)') 'FNCS failed to initialize'
    stop
  end if

  call fncs_get_name ( my_name )
  fed_id = fncs_get_id()
  fed_size = fncs_get_simulator_count()

  write (*,'(A,A,A)') "My name is '", trim(my_name), "'"
  write (*,'(A,I2,A,I2,A)') 'I am federate ', fed_id,&
      ' out of ', fed_size, ' other federates'

  do time_requested = 2, 10, 2
    current_time = fncs_time_request ( time_requested )
    event_size = fncs_get_events_size ( )
    write (*,'(A,I2,A,I2,A)') "current time is ", current_time, ", received ", event_size, " events"
    write (*,'(A,A,A,A)') TAB,"event",TAB,"value"
    do event_index = 1, event_size
      call fncs_get_event_at ( event_index, event_name )
      call fncs_get_value ( event_name, event_value )
      write (*,'(A,A,A,A)') TAB,trim(event_name),TAB,trim(event_value)
    end do
    call fncs_publish ( "key", "value" )
    call fncs_publish_anon ( "global/key_anon", my_name )
    call fncs_route ( "from", "to", "key", "value" )
  end do

  call fncs_finalize ( )
  if ( fncs_is_initialized ( ) ) then
    write (*,'(A)') 'FNCS failed to finalize'
    stop
  end if

  stop
end
