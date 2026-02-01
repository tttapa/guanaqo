#!/usr/bin/env python3
"""Test guanaqo::MatrixView type caster for nanobind."""

import numpy as np
import pytest

import $<TARGET_FILE_BASE_NAME:guanaqo_matrix_view> as mv


def test_ptr_c():
    """Version with dynamic inner stride should never copy non-contiguous data."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="C")
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const(a)
    assert returned_ptr == original_data_ptr


def test_ptr_fortran():
    """Version with dynamic inner stride should never copy non-contiguous data."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="F")
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const(a)
    assert returned_ptr == original_data_ptr


def test_ptr_C_c():
    """Version with C layout should not copy data in C layout."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="C")
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_C(a)
    assert returned_ptr == original_data_ptr


def test_ptr_F_fortran():
    """Version with Fortran layout should not copy data in Fortran layout."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="F")
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_F(a)
    assert returned_ptr == original_data_ptr


def test_ptr_C_fortran():
    """Version with C layout should copy data in Fortran layout."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="F")
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_C(a)
    assert returned_ptr != original_data_ptr


def test_ptr_F_c():
    """Version with Fortran layout should copy data in C layout."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="C")
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_F(a)
    assert returned_ptr != original_data_ptr


def test_ptr_C_fortran():
    """Version with C layout should copy data in Fortran layout."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="F")
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_C(a)
    assert returned_ptr != original_data_ptr


def test_ptr_slice_c():
    """Version with dynamic inner stride should never copy non-contiguous data."""
    a = np.full((10, 10), 42.0, order="C")[2:6, 3:8]
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const(a)
    assert returned_ptr == original_data_ptr


def test_ptr_slice_fortran():
    """Version with dynamic inner stride should never copy non-contiguous data."""
    a = np.full((10, 10), 42.0, order="F")[2:6, 3:8]
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const(a)
    assert returned_ptr == original_data_ptr


def test_ptr_slice_C_c():
    """Version with C layout should not copy data in C layout."""
    a = np.full((10, 10), 42.0, order="C")[2:6, 3:8]
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_C(a)
    assert returned_ptr == original_data_ptr


def test_ptr_slice_F_fortran():
    """Version with Fortran layout should not copy data in Fortran layout."""
    a = np.full((10, 10), 42.0, order="F")[2:6, 3:8]
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_F(a)
    assert returned_ptr == original_data_ptr


def test_ptr_slice_C_fortran():
    """Version with C layout should copy data in Fortran layout."""
    a = np.full((10, 10), 42.0, order="F")[2:6, 3:8]
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_C(a)
    assert returned_ptr != original_data_ptr


def test_ptr_slice_F_c():
    """Version with Fortran layout should copy data in C layout."""
    a = np.full((10, 10), 42.0, order="C")[2:6, 3:8]
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_F(a)
    assert returned_ptr != original_data_ptr


def test_ptr_F4():
    """Version with non-unit inner stride should not copy data if stride matches."""
    a = np.full((40, 10), 42.0, order="F")[::4, :]
    original_data_ptr = a.__array_interface__["data"][0]
    returned_ptr = mv.ptr_const_F4(a)
    assert returned_ptr == original_data_ptr


def test_ptr_F4_invalid():
    """Version with non-unit inner stride should never copy different strides."""
    a = np.full((40, 10), 42.0, order="F")[::3, :]
    with pytest.raises(TypeError):
        mv.ptr_const_F4(a)
