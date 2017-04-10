module fncs
  use iso_c_binding, only : c_char, c_null_char

  implicit none

! C string terminator alais using the 3-letter ASCII name.
! The C_ prefix is not used because it is just an ASCII character.
  character(len=1,kind=c_char), parameter :: NUL = c_null_char

  private c_f_string

  interface

    subroutine fncs_initialize ( ) bind ( c, name = "fncs_initialize" )
    end subroutine fncs_initialize

    subroutine fncs_initialize_config_ptr ( config ) bind ( c, name = "fncs_initialize_config" )
      use iso_c_binding, only : c_char
      character ( kind = c_char ), intent ( in ) :: config ( * )
    end subroutine fncs_initialize_config_ptr

    function fncs_is_initialized ( ) bind ( c, name = "fncs_is_initialized" )
      use iso_c_binding, only : c_int
      integer ( kind = c_int ) :: fncs_is_initialized
    end function fncs_is_initialized

    function fncs_time_request ( next ) bind ( c, name = "fncs_time_request" )
      use iso_c_binding, only : c_int64_t
      integer ( kind = c_int64_t ), value :: next
      integer ( kind = c_int64_t ) :: fncs_time_request
    end function fncs_time_request

    subroutine fncs_publish_ptr ( key, value_ ) bind ( c, name = "fncs_publish" )
      use iso_c_binding, only : c_char
      character ( kind = c_char ), intent ( in ) :: key ( * )
      character ( kind = c_char ), intent ( in ) :: value_ ( * )
    end subroutine fncs_publish_ptr

    subroutine fncs_publish_anon_ptr ( key, value_ ) bind ( c, name = "fncs_publish_anon" )
      use iso_c_binding, only : c_char
      character ( kind = c_char ), intent ( in ) :: key ( * )
      character ( kind = c_char ), intent ( in ) :: value_ ( * )
    end subroutine fncs_publish_anon_ptr

    subroutine fncs_route_ptr ( from, to, key, value_ ) bind ( c, name = "fncs_route" )
      use iso_c_binding, only : c_char
      character ( kind = c_char ), intent ( in ) :: from ( * )
      character ( kind = c_char ), intent ( in ) :: to ( * )
      character ( kind = c_char ), intent ( in ) :: key ( * )
      character ( kind = c_char ), intent ( in ) :: value_ ( * )
    end subroutine fncs_route_ptr

    subroutine fncs_die ( ) bind ( c, name = "fncs_die" )
    end subroutine fncs_die

    subroutine fncs_finalize ( ) bind ( c, name = "fncs_finalize" )
    end subroutine fncs_finalize

    subroutine fncs_update_time_delta ( delta ) bind ( c, name = "fncs_update_time_delta" )
      use iso_c_binding, only : c_int64_t
      integer ( kind = c_int64_t ), value, intent ( in ) :: delta
    end subroutine fncs_update_time_delta

    function fncs_get_events_size ( ) bind ( c, name = "fncs_get_events_size" )
      use iso_c_binding, only : c_int64_t
      integer ( kind = c_int64_t ) :: fncs_get_events_size
    end function fncs_get_events_size

    function fncs_get_event_at_ptr ( i ) bind ( c, name = "fncs_get_event_at" )
      use iso_c_binding, only : c_int64_t, c_ptr
      integer ( kind = c_int64_t ), value, intent ( in ) :: i
      type ( c_ptr ) :: fncs_get_event_at_ptr
    end function fncs_get_event_at_ptr

    function fncs_get_value_ptr ( key ) bind ( c, name = "fncs_get_value" )
      use iso_c_binding, only : c_char, c_ptr
      character ( kind = c_char ), intent ( in ) :: key ( * )
      type ( c_ptr ) :: fncs_get_value_ptr
    end function fncs_get_value_ptr

    function fncs_get_values_size ( ) bind ( c, name = "fncs_get_values_size" )
      use iso_c_binding, only : c_int64_t
      integer ( kind = c_int64_t ) :: fncs_get_values_size
    end function fncs_get_values_size

    function fncs_get_value_at_ptr ( key, i ) bind ( c, name = "fncs_get_value_at" )
      use iso_c_binding, only : c_int64_t, c_char, c_ptr
      character ( kind = c_char ), intent ( in ) :: key ( * )
      integer ( kind = c_int64_t ), value, intent ( in ) :: i
      type ( c_ptr ) :: fncs_get_value_at_ptr
    end function fncs_get_value_at_ptr

    function fncs_get_keys_size ( ) bind ( c, name = "fncs_get_keys_size" )
      use iso_c_binding, only : c_int64_t
      integer ( kind = c_int64_t ) :: fncs_get_keys_size
    end function fncs_get_keys_size

    function fncs_get_key_at_ptr ( i ) bind ( c, name = "fncs_get_key_at" )
      use iso_c_binding, only : c_int64_t, c_ptr
      integer ( kind = c_int64_t ), value, intent ( in ) :: i
      type ( c_ptr ) :: fncs_get_key_at_ptr
    end function fncs_get_key_at_ptr

    function fncs_get_name_ptr ( ) bind ( c, name = "fncs_get_name" )
      use iso_c_binding, only : c_ptr
      type ( c_ptr ) :: fncs_get_name_ptr
    end function fncs_get_name_ptr

    function fncs_get_id ( ) bind ( c, name = "fncs_get_id" )
      use iso_c_binding, only : c_int
      integer ( kind = c_int ) :: fncs_get_id
    end function fncs_get_id

    function fncs_get_simulator_count ( ) bind ( c, name = "fncs_get_simulator_count" )
      use iso_c_binding, only : c_int
      integer ( kind = c_int ) :: fncs_get_simulator_count
    end function fncs_get_simulator_count

    subroutine fncs_get_version ( major, minor, patch ) bind ( c, name = "fncs_get_version" )
      use iso_c_binding, only : c_int
      integer ( kind = c_int ), intent ( out ) :: major
      integer ( kind = c_int ), intent ( out ) :: minor
      integer ( kind = c_int ), intent ( out ) :: patch
    end subroutine fncs_get_version

  end interface

contains

  ! HACK: For some reason, c_associated was not defined as pure. 
  pure logical &
  function c_associated_pure(ptr) result(associated)
    use iso_c_binding, only: c_ptr, c_intptr_t
    type(c_ptr), intent(in) :: ptr
    integer(C_intptr_t) :: iptr
    iptr = transfer(ptr,iptr)
    associated = (iptr /= 0)
  end function c_associated_pure

! Set a fixed-length Fortran string to the value of a C string.
  subroutine F_string_assign_C_string(F_string, C_string)
    use iso_c_binding, only: c_ptr, c_char, c_f_pointer
    character(len=*), intent(out) :: F_string
    type(c_ptr), intent(in) :: C_string
    character(len=1,kind=c_char), pointer :: p_chars(:)
    integer :: i
    if (.not. c_associated_pure(C_string) ) then
      F_string = ' '
    else
      call c_f_pointer(C_string,p_chars,[huge(0)])
      i=1
      do while(p_chars(i)/=NUL .and. i<=len(F_string))
        F_string(i:i) = p_chars(i)
        i=i+1
      end do
      if (i<len(F_string)) F_string(i:) = ' '
    end if
  end subroutine F_string_assign_C_string

  subroutine c_f_string ( c_str, f_str )
    use iso_c_binding, only: c_ptr, c_char
    type ( c_ptr ), intent ( in ) :: c_str
    character(len=*), intent ( inout ) :: f_str
    interface
      ! steal std c library functions rather than writing our own.
      function strlen ( s ) bind ( c, name='strlen' )
        use iso_c_binding, only: c_ptr, c_size_t
        type ( c_ptr ), intent ( in ), value :: s
        integer ( c_size_t ) :: strlen
      end function strlen
      ! Ignore the return value of strncpy -> subroutine
      ! "restrict" is always assumed if we do not pass a pointer
      subroutine strncpy ( dest, src, n ) bind ( c, name='strncpy' )
        use iso_c_binding, only: c_char, c_ptr, c_size_t
        character ( kind = c_char ),  intent ( in ) :: dest ( * )
        type ( c_ptr ), intent ( in ), value :: src
        integer ( c_size_t ), value, intent ( in ) :: n
      end subroutine strncpy
    end interface
    call strncpy ( f_str, c_str, strlen ( c_str ) )
!    call F_string_assign_C_string ( f_str, c_str )
  end subroutine c_f_string

  subroutine fncs_initialize_config ( config )
    use iso_c_binding, only : c_null_char
    character ( len=* ), intent ( in ) :: config
    call fncs_initialize_config_ptr ( trim(config)//c_null_char )
  end subroutine

  subroutine fncs_publish ( key, value_ )
    use iso_c_binding, only : c_null_char
    character ( len=* ), intent ( in ) :: key
    character ( len=* ), intent ( in ) :: value_
    call fncs_publish_ptr ( trim(key)//c_null_char,&
                            trim(value_)//c_null_char )
  end subroutine

  subroutine fncs_publish_anon ( key, value_ )
    use iso_c_binding, only : c_null_char
    character ( len=* ), intent ( in ) :: key
    character ( len=* ), intent ( in ) :: value_
    call fncs_publish_anon_ptr ( trim(key)//c_null_char,&
                                 trim(value_)//c_null_char )
  end subroutine

  subroutine fncs_route ( from, to, key, value_ )
    use iso_c_binding, only : c_null_char
    character ( len=* ), intent ( in ) :: from
    character ( len=* ), intent ( in ) :: to
    character ( len=* ), intent ( in ) :: key
    character ( len=* ), intent ( in ) :: value_
    call fncs_route_ptr ( trim(from)//c_null_char,&
                          trim(to)//c_null_char,&
                          trim(key)//c_null_char,&
                          trim(value_)//c_null_char )
  end subroutine

  subroutine fncs_get_event_at ( i, f_string )
    use iso_c_binding, only : c_int64_t, c_ptr
    integer ( kind = c_int64_t ), intent ( in ) :: i
    character ( len=* ), intent ( out ) :: f_string
    type ( c_ptr ) :: c_string
    c_string = fncs_get_event_at_ptr ( i )
    call c_f_string ( c_string, f_string )
  end subroutine

  subroutine fncs_get_value ( key, f_string )
    use iso_c_binding, only : c_null_char, c_ptr
    character ( len=* ), intent ( in ) :: key
    character ( len=* ), intent ( out ) :: f_string
    type ( c_ptr ) :: c_string
    c_string = fncs_get_value_ptr ( trim(key)//c_null_char )
    call c_f_string ( c_string, f_string )
  end subroutine

  subroutine fncs_get_value_at ( key, i, f_string )
    use iso_c_binding, only : c_int64_t, c_null_char, c_ptr
    character ( len=* ), intent ( in ) :: key
    integer ( kind = c_int64_t ), intent ( in ) :: i
    character ( len=* ), intent ( out ) :: f_string
    type ( c_ptr ) :: c_string
    c_string = fncs_get_value_at_ptr ( trim(key)//c_null_char, i )
    call c_f_string ( c_string, f_string )
  end subroutine

  subroutine fncs_get_key_at ( i, f_string )
    use iso_c_binding, only : c_int64_t, c_ptr
    integer ( kind = c_int64_t ), intent ( in ) :: i
    character ( len=* ), intent ( out ) :: f_string
    type ( c_ptr ) :: c_string
    c_string = fncs_get_key_at_ptr ( i )
    call c_f_string ( c_string, f_string )
  end subroutine

  subroutine fncs_get_name ( f_string )
    use iso_c_binding, only : c_ptr
    character ( len=* ), intent (out) :: f_string
    type ( c_ptr ) :: c_string
    c_string = fncs_get_name_ptr ( )
    call c_f_string ( c_string, f_string )
  end subroutine

end module
