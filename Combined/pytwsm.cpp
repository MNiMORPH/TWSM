#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "TWSM.cpp"

namespace py = pybind11;

ArrayPack arp;


uint8_t test(uint8_t a){
    uint8_t out = a + 1;
    return (out);
}

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

void set_wtd(py::array_t<float> nparray){
    arp.wtd = _set_return_array(arp.wtd, nparray);
}

float get_wtd_at_index(int32_t i){
    return arp.wtd(i);
}

py::array_t<float> get_wtd(){
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
        { n_cols * sizeof(float), sizeof(float) }  /* Strides for each dimension */
    ));
    // Pointer for looping over output array
    py::buffer_info nparray_info = nparray.request();
    float *nparray_ptr = (float*) nparray_info.ptr; 
    for (uint32_t yi=0; yi<n_rows; yi++){
        for (uint32_t xi=0; xi<n_cols; xi++){
            nparray_ptr[yi*n_cols + xi] = arp.wtd(xi, yi);
        }
    }
    return nparray;
}


PYBIND11_MODULE(pytwsm, m) {
    // optional module docstring
    m.doc() = "pybind11 test";

    // define add function
    m.def("test", &test, "int++");
    m.def("set_wtd", &set_wtd, "");
    m.def("get_wtd", &get_wtd, "");
    m.def("get_wtd_at_index", &get_wtd_at_index, "");
}

