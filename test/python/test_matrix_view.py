#!/usr/bin/env python3
"""Test guanaqo::MatrixView type caster for nanobind."""

import numpy as np
import pytest

import guanaqo_matrix_view as mv


def test_times_two_fortran():
    """Test times_two with Fortran-ordered (column-major) array."""
    a = np.array([[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]], order="F")
    original = a.copy()
    mv.times_two(a)
    np.testing.assert_equal(a, original * 2.0)


def test_times_two_F_fortran():
    """Test times_two_F with Fortran-ordered array."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="F")
    original = a.copy()
    mv.times_two_F(a)
    np.testing.assert_equal(a, original * 2.0)


def test_times_two_C_c_order():
    """Test times_two_C with C-ordered (row-major) array."""
    a = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], order="C")
    original = a.copy()
    mv.times_two_C(a)
    np.testing.assert_equal(a, original * 2.0)


def test_view_modifies_original():
    """Verify that MatrixView modifies the original array in-place."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="F")
    original_data_ptr = a.__array_interface__["data"][0]
    mv.times_two_F(a)
    assert a.__array_interface__["data"][0] == original_data_ptr
    np.testing.assert_array_equal(a, [[2.0, 4.0], [6.0, 8.0]])


def test_empty_array():
    """Test with empty arrays."""
    a = np.array([], dtype=np.float64).reshape(0, 5, order="F")
    mv.times_two_F(a)
    assert a.shape == (0, 5)

    b = np.array([], dtype=np.float64).reshape(3, 0, order="F")
    mv.times_two_F(b)
    assert b.shape == (3, 0)


def test_single_element():
    """Test with single element arrays."""
    a = np.array([[5.0]], order="F")
    mv.times_two_F(a)
    assert a[0, 0] == 10.0


def test_row_vector():
    """Test with row vector."""
    a = np.array([[1.0, 2.0, 3.0]], order="F")
    mv.times_two_F(a)
    np.testing.assert_array_equal(a, [[2.0, 4.0, 6.0]])


def test_column_vector():
    """Test with column vector."""
    a = np.array([[1.0], [2.0], [3.0]], order="F")
    mv.times_two_F(a)
    np.testing.assert_array_equal(a, [[2.0], [4.0], [6.0]])


def test_reject_wrong_dtype_float32():
    """Reject float32 when float64 is expected."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], dtype=np.float32, order="F")
    with pytest.raises(TypeError):
        mv.times_two_F(a)


def test_reject_wrong_dtype_int():
    """Reject int when float64 is expected."""
    a = np.array([[1, 2], [3, 4]], dtype=np.int32, order="F")
    with pytest.raises(TypeError):
        mv.times_two_F(a)


def test_reject_wrong_order_c_to_f():
    """Reject C-order array when Fortran-order is expected."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="C")
    with pytest.raises(TypeError):
        mv.times_two_F(a)


def test_reject_wrong_order_f_to_c():
    """Reject Fortran-order array when C-order is expected."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="F")
    with pytest.raises(TypeError):
        mv.times_two_C(a)


def test_reject_1d_array():
    """Reject 1D arrays when 2D is expected."""
    a = np.array([1.0, 2.0, 3.0], dtype=np.float64)
    with pytest.raises(TypeError):
        mv.times_two_F(a)


def test_reject_3d_array():
    """Reject 3D arrays when 2D is expected."""
    a = np.array([[[1.0, 2.0], [3.0, 4.0]]], dtype=np.float64, order="F")
    with pytest.raises(TypeError):
        mv.times_two_F(a)


def test_reject_non_contiguous_inner():
    """Reject arrays with non-contiguous inner dimension."""
    a = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], order="F")
    sliced = a[:, ::2]
    mv.times_two_F(sliced)

    a_c = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], order="C")
    row_view = a_c[::2, :]
    with pytest.raises(TypeError):
        mv.times_two_F(row_view)


# Stride tests


def test_static_stride_4():
    """Test times_two_F4 with stride of 4."""
    base = np.zeros((16, 3), dtype=np.float64, order="F")
    view = base[::4, :]
    view[:] = [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [7.0, 8.0, 9.0], [10.0, 11.0, 12.0]]

    assert view.strides[0] == 32
    original = view.copy()
    mv.times_two_F4(view)
    np.testing.assert_equal(view, original * 2.0)


def test_reject_wrong_static_stride():
    """Reject arrays that don't have the required static stride."""
    a = np.array([[1.0, 2.0], [3.0, 4.0]], order="F")
    with pytest.raises(TypeError):
        mv.times_two_F4(a)


def test_submatrix_both_dims_fortran():
    """Test with a submatrix view slicing both rows and columns in Fortran order."""
    large = np.array(
        [
            [1.0, 2.0, 3.0, 4.0, 5.0],
            [6.0, 7.0, 8.0, 9.0, 10.0],
            [11.0, 12.0, 13.0, 14.0, 15.0],
            [16.0, 17.0, 18.0, 19.0, 20.0],
        ],
        order="F",
    )

    sub = large[1:3, 1:4]
    original = sub.copy()
    mv.times_two_F(sub)
    np.testing.assert_equal(sub, original * 2.0)

    expected_modified = np.array([[7.0, 8.0, 9.0], [12.0, 13.0, 14.0]]) * 2.0
    np.testing.assert_equal(large[1:3, 1:4], expected_modified)

    assert large[0, 0] == 1.0
    assert large[0, 4] == 5.0
    assert large[3, 0] == 16.0
    assert large[3, 4] == 20.0


def test_submatrix_both_dims_c_order():
    """Test with a submatrix view slicing both rows and columns in C order."""
    large = np.array(
        [
            [1.0, 2.0, 3.0, 4.0, 5.0],
            [6.0, 7.0, 8.0, 9.0, 10.0],
            [11.0, 12.0, 13.0, 14.0, 15.0],
            [16.0, 17.0, 18.0, 19.0, 20.0],
        ],
        order="C",
    )

    sub = large[1:3, 1:4]
    original = sub.copy()
    mv.times_two_C(sub)
    np.testing.assert_equal(sub, original * 2.0)

    expected_modified = np.array([[7.0, 8.0, 9.0], [12.0, 13.0, 14.0]]) * 2.0
    np.testing.assert_equal(large[1:3, 1:4], expected_modified)

    assert large[0, 0] == 1.0
    assert large[0, 4] == 5.0
    assert large[3, 0] == 16.0
    assert large[3, 4] == 20.0
