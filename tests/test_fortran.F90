program main
  use fncs
  use iso_c_binding, only : C_CHAR, C_NULL_CHAR
  implicit none
  integer ( 8 ) time
  integer ( 8 ) event_size
  integer major
  integer minor
  integer patch

  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) 'HELLO_PRB'
  write ( *, '(a)' ) '  FORTRAN90 version'
  write ( *, '(a)' ) '  Demonstrate how a FORTRAN90 program can call'
  write ( *, '(a)' ) '  a C function to print a character string.'
!
!  It is probably acceptable to replace C_CHAR_"Hello World!"
!  by just "Hello World!", but this is not actually guaranteed
!  by the FORTRAN standard.
!
  time = 0
  call fncs_get_version ( major, minor, patch )
  call fncs_initialize ( )
  call fncs_publish ( C_CHAR_'key'//C_NULL_CHAR, &
                      C_CHAR_'value'//C_NULL_CHAR )
  time = fncs_time_request ( time )
  event_size = fncs_get_events_size ( )
  write (*,*) major, minor, patch
  write ( *,* ) ' ', event_size
  call fncs_finalize ( )
!
!  Terminate.
!
  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) 'HELLO_PRB:'
  write ( *, '(a)' ) '  Normal end of execution.'
  write ( *, '(a)' ) ' '

  stop
end
