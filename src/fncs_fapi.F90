module fncs
  implicit none
  private c_f_string
  private get_scalar_pointer

  interface

    subroutine fncs_initialize ( ) bind ( c, name = "fncs_initialize" )
    end subroutine fncs_initialize

    subroutine fncs_initialize_config ( config ) bind ( c, name = "fncs_initialize_config" )
      use iso_c_binding, only : c_char
      character ( kind = c_char ) :: config ( * )
    end subroutine fncs_initialize_config

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
      character ( kind = c_char ) :: key ( * )
      character ( kind = c_char ) :: value_ ( * )
    end subroutine fncs_publish_ptr

    subroutine fncs_publish_anon_ptr ( key, value_ ) bind ( c, name = "fncs_publish_anon" )
      use iso_c_binding, only : c_char
      character ( kind = c_char ) :: key ( * )
      character ( kind = c_char ) :: value_ ( * )
    end subroutine fncs_publish_anon_ptr

    subroutine fncs_route_ptr ( from, to, key, value_ ) bind ( c, name = "fncs_route" )
      use iso_c_binding, only : c_char
      character ( kind = c_char ) :: from ( * )
      character ( kind = c_char ) :: to ( * )
      character ( kind = c_char ) :: key ( * )
      character ( kind = c_char ) :: value_ ( * )
    end subroutine fncs_route_ptr

    subroutine fncs_die ( ) bind ( c, name = "fncs_die" )
    end subroutine fncs_die

    subroutine fncs_finalize ( ) bind ( c, name = "fncs_finalize" )
    end subroutine fncs_finalize

    subroutine fncs_update_time_delta ( delta ) bind ( c, name = "fncs_update_time_delta" )
      use iso_c_binding, only : c_int64_t
      integer ( kind = c_int64_t ), value :: delta
    end subroutine fncs_update_time_delta

    function fncs_get_events_size ( ) bind ( c, name = "fncs_get_events_size" )
      use iso_c_binding, only : c_int64_t
      integer ( kind = c_int64_t ) :: fncs_get_events_size
    end function fncs_get_events_size

    function fncs_get_event_at_ptr ( i ) bind ( c, name = "fncs_get_event_at" )
      use iso_c_binding, only : c_int64_t, c_ptr
      integer ( kind = c_int64_t ), value :: i
      type ( c_ptr ) :: fncs_get_event_at_ptr
    end function fncs_get_event_at_ptr

    function fncs_get_value_ptr ( key ) bind ( c, name = "fncs_get_value" )
      use iso_c_binding, only : c_char, c_ptr
      character ( kind = c_char ) :: key ( * )
      type ( c_ptr ) :: fncs_get_value_ptr
    end function fncs_get_value_ptr

    function fncs_get_values_size ( ) bind ( c, name = "fncs_get_values_size" )
      use iso_c_binding, only : c_int64_t
      integer ( kind = c_int64_t ) :: fncs_get_values_size
    end function fncs_get_values_size

    function fncs_get_value_at_ptr ( key, i ) bind ( c, name = "fncs_get_value_at" )
      use iso_c_binding, only : c_int64_t, c_char, c_ptr
      character ( kind = c_char ) :: key ( * )
      integer ( kind = c_int64_t ), value :: i
      type ( c_ptr ) :: fncs_get_value_at_ptr
    end function fncs_get_value_at_ptr

    function fncs_get_keys_size ( ) bind ( c, name = "fncs_get_keys_size" )
      use iso_c_binding, only : c_int64_t
      integer ( kind = c_int64_t ) :: fncs_get_keys_size
    end function fncs_get_keys_size

    function fncs_get_key_at_ptr ( i ) bind ( c, name = "fncs_get_key_at" )
      use iso_c_binding, only : c_int64_t, c_ptr
      integer ( kind = c_int64_t ), value :: i
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

  function c_f_string(c_str) result(f_str)
    use, intrinsic :: iso_c_binding, only: c_ptr, c_f_pointer, c_char
    type(c_ptr), intent(in) :: c_str
    character(:,kind=c_char), pointer :: f_str
    character(kind=c_char), pointer :: arr(:)
    interface
      ! steal std c library function rather than writing our own.
      function strlen(s) bind(c, name='strlen')
        use, intrinsic :: iso_c_binding, only: c_ptr, c_size_t
        implicit none
        !----
        type(c_ptr), intent(in), value :: s
        integer(c_size_t) :: strlen
      end function strlen
    end interface
    !****
    call c_f_pointer(c_str, arr, [strlen(c_str)])
    call get_scalar_pointer(size(arr), arr, f_str)
  end function c_f_string

  subroutine get_scalar_pointer(scalar_len, scalar, ptr)
    use, intrinsic :: iso_c_binding, only: c_char
    integer, intent(in) :: scalar_len
    character(kind=c_char,len=scalar_len), intent(in), target :: scalar(1)
    character(:,kind=c_char), intent(out), pointer :: ptr
    !***
    ptr => scalar(1)
  end subroutine get_scalar_pointer

  subroutine fncs_publish ( key, value_ )
    use iso_c_binding, only : c_null_char
    character ( len=* ) :: key
    character ( len=* ) :: value_

    call fncs_publish_ptr ( trim(key)//c_null_char,&
                            trim(value_)//c_null_char )
  end subroutine

  subroutine fncs_publish_anon ( key, value_ )
    use iso_c_binding, only : c_null_char
    character ( len=* ) :: key
    character ( len=* ) :: value_

    call fncs_publish_anon_ptr ( trim(key)//c_null_char,&
                                 trim(value_)//c_null_char )
  end subroutine

  subroutine fncs_route ( from, to, key, value_ )
    use iso_c_binding, only : c_null_char
    character ( len=* ) :: from
    character ( len=* ) :: to
    character ( len=* ) :: key
    character ( len=* ) :: value_

    call fncs_route_ptr ( trim(from)//c_null_char,&
                          trim(to)//c_null_char,&
                          trim(key)//c_null_char,&
                          trim(value_)//c_null_char )
  end subroutine

  function fncs_get_event_at ( i ) result ( string )
    use iso_c_binding, only : c_int64_t, c_ptr
    integer ( kind = c_int64_t ) :: i
    character, pointer, dimension ( : ) :: string

    type ( c_ptr ) :: c_string

    c_string = fncs_get_event_at_ptr ( i )
    string = c_f_string ( c_string )
  end function

  function fncs_get_value ( key ) result ( string )
    use iso_c_binding, only : c_null_char, c_ptr
    character ( len=* ) :: key
    character, pointer, dimension ( : ) :: string

    type ( c_ptr ) :: c_string

    c_string = fncs_get_value_ptr ( trim(key)//c_null_char )
    string = c_f_string ( c_string )
  end function

  function fncs_get_value_at ( key, i ) result ( string )
    use iso_c_binding, only : c_int64_t, c_null_char, c_ptr
    character ( len=* ) :: key
    integer ( kind = c_int64_t ) :: i
    character, pointer, dimension ( : ) :: string

    type ( c_ptr ) :: c_string

    c_string = fncs_get_value_at_ptr ( trim(key)//c_null_char, i )
    string = c_f_string ( c_string )
  end function

  function fncs_get_key_at ( i ) result ( string )
    use iso_c_binding, only : c_int64_t, c_ptr
    integer ( kind = c_int64_t ) :: i
    character, pointer, dimension ( : ) :: string

    type ( c_ptr ) :: c_string

    c_string = fncs_get_key_at_ptr ( i )
    string = c_f_string ( c_string )
  end function

  function fncs_get_name ( ) result ( string )
    use iso_c_binding, only : c_ptr
    character, pointer, dimension ( : ) :: string

    type ( c_ptr ) :: c_string

    c_string = fncs_get_name_ptr ( )
    string = c_f_string ( c_string )
  end function

end module
