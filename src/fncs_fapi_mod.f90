module fncs
    interface
        subroutine fncs_initialize ( )
        end subroutine
        subroutine fncs_initialize_config ( config )
            character ( len = * ), intent ( in ) :: config
        end subroutine
        logical function fncs_is_initialized ( )
        end function
        integer*8 function fncs_time_request ( next )
            integer*8, intent ( in ) :: next
        end function
        subroutine fncs_publish ( key, val )
            character ( len = * ), intent ( in ) :: key
            character ( len = * ), intent ( in ) :: val
        end subroutine
        subroutine fncs_publish_anon ( key, val )
            character ( len = * ), intent ( in ) :: key
            character ( len = * ), intent ( in ) :: val
        end subroutine
        subroutine fncs_route ( from, to, key, val )
            character ( len = * ), intent ( in ) :: from
            character ( len = * ), intent ( in ) :: to
            character ( len = * ), intent ( in ) :: key
            character ( len = * ), intent ( in ) :: val
        end subroutine
        subroutine fncs_die ( )
        end subroutine
        subroutine fncs_finalize ( )
        end subroutine
        subroutine fncs_update_time_delta ( delta )
            integer*8, intent ( in ) :: delta
        end subroutine
        integer*8 function fncs_get_events_size ( )
        end function
        subroutine fncs_get_event_at ( event_index, event )
            integer*8, intent ( in ) :: event_index
            character ( len = * ), intent ( out ) :: event
        end subroutine
        subroutine fncs_get_value ( key, val )
            character ( len = * ), intent ( in ) :: key
            character ( len = * ), intent ( out ) :: val
        end subroutine
        integer*8 function fncs_get_values_size ( )
        end function
        subroutine fncs_get_value_at ( key, key_index, val )
            character ( len = * ), intent ( in ) :: key
            integer*8, intent ( in ) :: key_index
            character ( len = * ), intent ( out ) :: val
        end subroutine
        subroutine fncs_get_name ( my_name )
            character ( len = * ), intent ( out ) :: my_name
        end subroutine
        integer function fncs_get_id ( )
        end function
        integer function fncs_get_simulator_count ( )
        end function
        subroutine fncs_get_version ( major, minor, patch )
            integer, intent ( out ) :: major
            integer, intent ( out ) :: minor
            integer, intent ( out ) :: patch
        end subroutine
    end interface
end module
