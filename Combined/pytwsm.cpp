#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <string>
#include <iostream>

#include "TWSM.cpp"

namespace py = pybind11;

ArrayPack arp;


uint8_t test(uint8_t a){
    uint8_t out = a + 1;
    return (out);
}

///////////////////////////
// DECLARE SWITCH VALUES //
///////////////////////////

const uint8_t _WTD = 1;
const uint8_t _FDEPTH = 2;
const uint8_t _AIR_TEMPERATURE = 3;
const uint8_t _WINTER_AIR_TEMPERATURE = 4;
// missing 5!
const uint8_t _RELATIVE_HUMIDITY = 6;
const uint8_t _TOPOGRAPHIC_SLOPE = 7;
const uint8_t _STARTING_EVAPORATION = 8;
const uint8_t _EVAPORATION = 9;
const uint8_t _GROUND_TEMPERATURE = 10;
const uint8_t _WIND_SPEED = 11;

///////////////////////
// UTILITY FUNCTIONS //
///////////////////////

rd::Array2D<float> _set_return_array(rd::Array2D<float> arparray, \
                              py::array_t<float> nparray){
    /**
    Private utility function to return a RichDEM 2D array object that is set
    to be the same as the input numpy array.

    This will be called by each relevant setter to assign each array's
    dimensions and the values of its elements.

    @arparray The ArrayPack array object
    @nparray The NumPy array being passed
    **/

    // Default value for unassigned array elements
    //float NO_VALUE  = std::numeric_limits<float>::min();
    // Instantiate an ArrayPack object to access those data
    // Obtain nparray dimensions
    py::buffer_info nparray_info = nparray.request();
    uint32_t n_rows = nparray_info.shape.at(0);
    uint32_t n_cols = nparray_info.shape.at(1);
    // Pointer for looping over numpy array
    float *nparray_ptr = (float *) nparray_info.ptr;
    // Set array-pack array dimensions and default value
    arparray = rd::Array2D<float>(n_cols, n_rows, -100);// NO_VALUE);
    // Set values in arparray to equal those in nparray
    for (uint32_t yi=0; yi<n_rows; yi++){
        for (uint32_t xi=0; xi<n_cols; xi++){
            arparray(xi, yi) = nparray_ptr[yi*n_cols + xi];
        }
    }
    return arparray;
}

py::array_t<float> _get_array_elements(rd::Array2D<float> arparray){
    // Instantiate an ArrayPack object to access data on the C++ side
    // Copy the values into a Python array object
    // This adds some inefficiency
    uint32_t n_rows = 2;
    uint32_t n_cols = 3;
    auto nparray = py::array(py::buffer_info(
        nullptr,     /* Pointer to data (nullptr -> ask NumPy to allocate!) */
        sizeof(float),     /* Size of one item */
        py::format_descriptor<float>::value, /* Buffer format */
        2,          /* How many dimensions? */
        { n_rows, n_cols },  /* Number of elements for each dimension */
        { n_cols * sizeof(float), sizeof(float) }  /* Strides per dimension */
    ));
    // Pointer for looping over output array
    py::buffer_info nparray_info = nparray.request();
    float *nparray_ptr = (float*) nparray_info.ptr;
    for (uint32_t yi=0; yi<n_rows; yi++){
        for (uint32_t xi=0; xi<n_cols; xi++){
            nparray_ptr[yi*n_cols + xi] = arparray(xi, yi);
        }
    }
    return nparray;
}

py::array_t<float> _get_array(uint8_t _varcode ){
    // Instantiate an ArrayPack object to access data on the C++ side
    // Copy the values into a Python array object
    // This adds some inefficiency
    //f2d _vararray; // f2d is shorthand for rd::Array2D<float>
    switch( _varcode ){
        case _WTD:
            return _get_array_elements(arp.wtd);
        case _FDEPTH:
            return _get_array_elements(arp.fdepth);
        case _AIR_TEMPERATURE:
            return _get_array_elements(arp.temp);
        case _WINTER_AIR_TEMPERATURE:
            return _get_array_elements(arp.winter_temp);
        case _RELATIVE_HUMIDITY:
            return _get_array_elements(arp.relhum);
        case _TOPOGRAPHIC_SLOPE:
            return _get_array_elements(arp.slope);
        case _STARTING_EVAPORATION:
            return _get_array_elements(arp.starting_evap);
        case _EVAPORATION:
            return _get_array_elements(arp.evap);
        case _GROUND_TEMPERATURE:
            return _get_array_elements(arp.ground_temp);
        case _WIND_SPEED:
            return _get_array_elements(arp.wind_speed);
    }
}

///////////////////////////////
// ARRAY GETTERS AND SETTERS //
///////////////////////////////

// Water-table depth

void set_wtd(py::array_t<float> nparray){
    arp.wtd = _set_return_array(arp.wtd, nparray);
}

float get_wtd_at_index(int32_t i){
    return arp.wtd(i);
}

py::array_t<float> get_wtd(){
    return _get_array( _WTD );
}

// e-folding "f" depth

void set_fdepth(py::array_t<float> nparray){
    arp.fdepth = _set_return_array(arp.fdepth, nparray);
}

float get_fdepth_at_index(int32_t i){
    return arp.fdepth(i);
}

py::array_t<float> get_fdepth(){
    return _get_array( _FDEPTH );
}

// Air temperature

void set_temp(py::array_t<float> nparray){
    arp.temp = _set_return_array(arp.temp, nparray);
}

float get_temp_at_index(int32_t i){
    return arp.temp(i);
}

py::array_t<float> get_temp(){
    return _get_array( _AIR_TEMPERATURE );
}

// Winter air temperature

void set_winter_temp(py::array_t<float> nparray){
    arp.winter_temp = _set_return_array(arp.winter_temp, nparray);
}

float get_winter_temp_at_index(int32_t i){
    return arp.winter_temp(i);
}

py::array_t<float> get_winter_temp(){
    return _get_array( _WINTER_AIR_TEMPERATURE );
}

// Relative humidity

void set_relhum(py::array_t<float> nparray){
    arp.relhum = _set_return_array(arp.relhum, nparray);
}

float get_relhum_at_index(int32_t i){
    return arp.relhum(i);
}

py::array_t<float> get_relhum(){
    return _get_array( _RELATIVE_HUMIDITY );
}

// Slope

void set_slope(py::array_t<float> nparray){
    arp.slope = _set_return_array(arp.slope, nparray);
}

float get_slope_at_index(int32_t i){
    return arp.slope(i);
}

py::array_t<float> get_slope(){
    return _get_array( _TOPOGRAPHIC_SLOPE );
}

// Evaporation

void set_starting_evap(py::array_t<float> nparray){
    arp.starting_evap = _set_return_array(arp.starting_evap, nparray);
}

void set_evap(py::array_t<float> nparray){
    arp.evap = _set_return_array(arp.evap, nparray);
}

float get_starting_evap_at_index(int32_t i){
    return arp.starting_evap(i);
}

float get_evap_at_index(int32_t i){
    return arp.evap(i);
}

py::array_t<float> get_starting_evap(){
    return _get_array( _STARTING_EVAPORATION );
}

py::array_t<float> get_evap(){
    return _get_array( _EVAPORATION );
}

// Ground temperature

void set_ground_temp(py::array_t<float> nparray){
    arp.ground_temp = _set_return_array(arp.ground_temp, nparray);
}

float get_ground_temp_at_index(int32_t i){
    return arp.ground_temp(i);
}

py::array_t<float> get_ground_temp(){
    return _get_array( _GROUND_TEMPERATURE );
}

// Wind speed

void set_wind_speed(py::array_t<float> nparray){
    arp.wind_speed = _set_return_array(arp.wind_speed, nparray);
}

float get_wind_speed_at_index(int32_t i){
    return arp.wind_speed(i);
}

py::array_t<float> get_wind_speed(){
    return _get_array( _WIND_SPEED );
}


////////////////////////////
// EXPOSE PYTHON BINDINGS //
////////////////////////////

PYBIND11_MODULE(pytwsm, m) {
    // optional module docstring
    m.doc() = "pybind11 test";

    // define add function
    //m.def("test", &test, "int++");
    m.def("set_wtd", &set_wtd, "");
    m.def("get_wtd_at_index", &get_wtd_at_index, "");
    m.def("get_wtd", &get_wtd, "Return the water-table depth array",
          py::return_value_policy::reference);
    m.def("set_fdepth", &set_fdepth, "");
    m.def("get_fdepth", &get_fdepth, "",
          py::return_value_policy::reference);
    m.def("set_temp", &set_temp, "");
    m.def("get_temp", &get_temp, "",
          py::return_value_policy::reference);
    m.def("set_winter_temp", &set_winter_temp, "");
    m.def("get_winter_temp", &get_winter_temp, "",
          py::return_value_policy::reference);
    m.def("set_relhum", &set_relhum, "");
    m.def("get_relhum", &get_relhum, "",
          py::return_value_policy::reference);
    m.def("set_slope", &set_slope, "");
    m.def("get_slope", &get_slope, "",
          py::return_value_policy::reference);
    m.def("set_starting_evap", &set_starting_evap, "");
    m.def("get_starting_evap", &get_starting_evap, "",
          py::return_value_policy::reference);
    m.def("set_evap", &set_evap, "");
    m.def("get_evap", &get_evap, "",
          py::return_value_policy::reference);
    m.def("set_ground_temp", &set_ground_temp, "ground_temp");
    m.def("get_ground_temp", &get_ground_temp, "",
          py::return_value_policy::reference);
    m.def("set_wind_speed", &set_wind_speed, "");
    m.def("get_wind_speed", &get_wind_speed, "",
          py::return_value_policy::reference);
}
