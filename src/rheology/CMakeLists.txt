target_sources(${amr_wind_lib_name}
   PRIVATE
      #C++
      incflo_rheology.cpp
      incflo_read_rheology_parameters.cpp      
   )

target_include_directories(${amr_wind_lib_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
