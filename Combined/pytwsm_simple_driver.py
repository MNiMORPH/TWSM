import pytwsm
import numpy as np

a = np.array([[1.,2.,3.],[4.,5.,6.]])
pytwsm.set_wtd(a)
out = pytwsm.get_wtd__testing()
print(out)

# Works at last! Had not realized that I needed to define the full memory
# strides for each higher dimension.
a = np.array([[1.,2.,3.],[4.,5.,6.]])
pytwsm.set_wtd(a)
out = pytwsm.get_wtd()
print(out)

# INDICES ALL CORRECT FOR RICHDEM ARRAY!
a = np.array([[1.,2.,3.],[4.,5.,6.]])
pytwsm.set_wtd(a)
for i in range(6):
    print(pytwsm.get_wtd_at_index(i))

#a = np.zeros((2,3))
#pytwsm.test_set_wtd(a)

a = np.array([[1.,2.,3.],[4.,5.,6.]])
pytwsm.test2(a)






# scratch space

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
    float NO_VALUE  = std::numeric_limits<float>::min();
    // Instantiate an ArrayPack object to access those data
    // Obtain nparray dimensions
    py::buffer_info nparray_info = nparray.request();
    uint32_t n_rows = 2; //nparray_info.shape.at(0);
    uint32_t n_cols = 3; //nparray_info.shape.at(1);
    // Pointer for looping over numpy array
    float *nparray_ptr = (float *) nparray_info.ptr;
    // Set array-pack array dimensions and default value
    arparray = rd::Array2D<float>(n_cols, n_rows, -100);// NO_VALUE);
    // Set values in arparray to equal those in nparray
    uint8_t counter = 0;
    for (uint32_t yi=0; yi<n_rows; yi++){
        for (uint32_t xi=0; xi<n_cols; xi++){
            arparray(xi, yi) = nparray_ptr[counter]; //nparray_ptr[yi*3 + xi];
            counter++;
        }
    }
    return arparray;
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
        { sizeof(float), sizeof(float) }  /* Strides for each dimension */
    ));
    // Pointer for looping over output array
    py::buffer_info nparray_info = nparray.request();
    float *nparray_ptr = (float *) nparray_info.ptr; 
    uint8_t counter = 0;
    for (uint32_t yi=0; yi<n_rows; yi++){
        for (uint32_t xi=0; xi<n_cols; xi++){
            nparray_ptr[counter] = arp.wtd(counter);
            //nparray_ptr[yi*n_cols + xi] = arp.wtd(xi, yi); // arp.wtd(0, 0); //100.; //arp.wtd(yi, xi);
            counter++;
        }
    }
    return nparray;
    //return arp.wtd.size(); //(0, 0); //nparray; // (ssize_t) dims.size();
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
        { sizeof(float), sizeof(float) }  /* Strides for each dimension */
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

namespace py = py



    // test array -- works!
    constexpr size_t size = 2*3;
    float *foo = new float[size];
    for (size_t i = 0; i < size; i++) {
        foo[i] = (float) i;
    }
    // So I just need to extract the data array from within the RichDEM object
    
    py::array_t<float> nparray = py::array_t<float>(
        { n_rows, n_cols },  /* Number of elements for each dimension */
        { sizeof(float) * n_cols, sizeof(float) },  /* Strides for each dimension */
        foo     /* Data pointer */
    );
    return nparray;
