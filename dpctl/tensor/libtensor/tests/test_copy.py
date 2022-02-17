import numpy as np
import pytest

import dpctl
import dpctl.tensor as dpt
import dpctl.tensor._tensor_impl as ti

_usm_types_list = ["shared", "device", "host"]
_typestrs_list = [
    "b1",
    "u1",
    "i1",
    "u2",
    "i2",
    "u4",
    "i4",
    "u8",
    "i8",
    "f2",
    "f4",
    "f8",
    "c8",
    "c16",
]


@pytest.fixture(params=_usm_types_list)
def usm_type(request):
    return request.param


@pytest.fixture(params=_typestrs_list)
def src_typestr(request):
    return request.param


@pytest.fixture(params=_typestrs_list)
def dst_typestr(request):
    return request.param


def _random_vector(n, src_dt):
    src_dt = np.dtype(src_dt)
    if np.issubdtype(src_dt, np.integer):
        Xnp = np.random.randint(0, 2, size=n).astype(src_dt)
    elif np.issubdtype(src_dt, np.floating):
        Xnp = np.random.randn(n).astype(src_dt)
    elif np.issubdtype(src_dt, np.complexfloating):
        Xnp = np.random.randn(n) + 1j * np.random.randn(n)
        Xnp = Xnp.astype(src_dt)
    else:
        Xnp = np.random.randint(0, 2, size=n).astype(src_dt)
    return Xnp


def _force_cast(Xnp, dst_dt):
    if np.issubdtype(Xnp.dtype, np.complexfloating) and not np.issubdtype(
        dst_dt, np.complexfloating
    ):
        R = Xnp.real.astype(dst_dt, casting="unsafe", copy=True)
    else:
        R = Xnp.astype(dst_dt, casting="unsafe", copy=True)
    return R


def are_close(X1, X2):
    if np.issubdtype(X2.dtype, np.floating) or np.issubdtype(
        X2.dtype, np.complexfloating
    ):
        return np.allclose(X1, X2, atol=np.finfo(X2.dtype).eps)
    else:
        return np.allclose(X1, X2)


def test_copy1d_c_contig(src_typestr, dst_typestr):
    try:
        q = dpctl.SyclQueue()
    except dpctl.SyclQueueCreationError:
        pytest.skip("Queue could not be created")
    src_dt = np.dtype(src_typestr)
    dst_dt = np.dtype(dst_typestr)
    Xnp = _random_vector(4096, src_dt)

    X = dpt.asarray(Xnp, sycl_queue=q)
    Y = dpt.empty(Xnp.shape, dtype=dst_typestr, sycl_queue=q)
    ev = ti._copy_usm_ndarray_into_usm_ndarray(src=X, dst=Y, queue=q)
    ev.wait()
    Ynp = _force_cast(Xnp, dst_dt)
    assert are_close(Ynp, dpt.asnumpy(Y))
    q.wait()


def test_copy1d_strided(src_typestr, dst_typestr):
    try:
        q = dpctl.SyclQueue()
    except dpctl.SyclQueueCreationError:
        pytest.skip("Queue could not be created")
    src_dt = np.dtype(src_typestr)
    dst_dt = np.dtype(dst_typestr)
    Xnp = _random_vector(4096, src_dt)

    for s in (
        slice(None, None, 2),
        slice(None, None, -2),
    ):
        X = dpt.asarray(Xnp, sycl_queue=q)[s]
        Y = dpt.empty(X.shape, dtype=dst_typestr, sycl_queue=q)
        ev = ti._copy_usm_ndarray_into_usm_ndarray(src=X, dst=Y, queue=q)
        ev.wait()
        Ynp = _force_cast(Xnp[s], dst_dt)
        assert are_close(Ynp, dpt.asnumpy(Y))

    # now 0-strided source
    X = dpt.usm_ndarray((4096,), dtype=src_typestr, strides=(0,))
    X[0] = Xnp[0]
    Y = dpt.empty(X.shape, dtype=dst_typestr, sycl_queue=q)
    ti._copy_usm_ndarray_into_usm_ndarray(src=X, dst=Y, queue=q).wait()
    Ynp = _force_cast(np.broadcast_to(Xnp[0], X.shape), dst_dt)
    assert are_close(Ynp, dpt.asnumpy(Y))
    q.wait()


def test_copy1d_strided2(src_typestr, dst_typestr):
    try:
        q = dpctl.SyclQueue()
    except dpctl.SyclQueueCreationError:
        pytest.skip("Queue could not be created")
    src_dt = np.dtype(src_typestr)
    dst_dt = np.dtype(dst_typestr)
    Xnp = _random_vector(4096, src_dt)

    for s in (
        slice(None, None, 2),
        slice(None, None, -2),
    ):
        X = dpt.asarray(Xnp, sycl_queue=q)[s]
        Y = dpt.empty(X.shape, dtype=dst_typestr, sycl_queue=q)[::-1]
        ev = ti._copy_usm_ndarray_into_usm_ndarray(src=X, dst=Y, queue=q)
        ev.wait()
        Ynp = _force_cast(Xnp[s], dst_dt)
        assert are_close(Ynp, dpt.asnumpy(Y))
    q.wait()


def test_copy2d(src_typestr, dst_typestr):
    try:
        q = dpctl.SyclQueue()
    except dpctl.SyclQueueCreationError:
        pytest.skip("Queue could not be created")

    src_dt = np.dtype(src_typestr)
    dst_dt = np.dtype(dst_typestr)
    n1, n2 = 45, 78
    strides1 = [5, 3, 1]
    strides2 = [1, 2]
    for sgn1 in [-1, 1]:
        for sgn2 in [-1, 1]:
            for st1 in strides1:
                for st2 in strides2:
                    Snp = _random_vector(st1 * st2 * n1 * n2, src_dt).reshape(
                        (st1 * n1, st2 * n2)
                    )
                    Xnp = Snp[
                        slice(None, None, st1 * sgn1),
                        slice(None, None, st2 * sgn2),
                    ]
                    S = dpt.asarray(Snp, sycl_queue=q)
                    X = S[
                        slice(None, None, st1 * sgn1),
                        slice(None, None, st2 * sgn2),
                    ]
                    Y = dpt.empty((n1, n2), dtype=dst_dt)
                    ti._copy_usm_ndarray_into_usm_ndarray(
                        src=X, dst=Y, queue=q
                    ).wait()
                    Ynp = _force_cast(Xnp, dst_dt)
                    assert are_close(Ynp, dpt.asnumpy(Y))
                    Yst = dpt.empty((2 * n1, n2), dtype=dst_dt)[::2, ::-1]
                    ev = ti._copy_usm_ndarray_into_usm_ndarray(
                        src=X, dst=Yst, queue=q
                    )
                    Y = dpt.empty((n1, n2), dtype=dst_dt)
                    ti._copy_usm_ndarray_into_usm_ndarray(
                        src=Yst, dst=Y, queue=q, depends=[ev]
                    ).wait()
                    Ynp = _force_cast(Xnp, dst_dt)
                    assert are_close(Ynp, dpt.asnumpy(Y))


def test_copy3d(src_typestr, dst_typestr):
    try:
        q = dpctl.SyclQueue()
    except dpctl.SyclQueueCreationError:
        pytest.skip("Queue could not be created")

    src_dt = np.dtype(src_typestr)
    dst_dt = np.dtype(dst_typestr)
    n1, n2, n3 = 15, 14, 6
    strides1 = [5, 3, 1]
    strides2 = [1, 2]
    strides3 = [1, 2, 3]
    for sgn1 in [-1, 1]:
        for sgn2 in [-1, 1]:
            for sgn3 in [-1, 1]:
                for st1 in strides1:
                    for st2 in strides2:
                        for st3 in strides3:
                            Snp = _random_vector(
                                st1 * st2 * st3 * n1 * n2 * n3, src_dt
                            ).reshape((st1 * n1, st2 * n2, st3 * n3))
                            Xnp = Snp[
                                slice(None, None, st1 * sgn1),
                                slice(None, None, st2 * sgn2),
                                slice(None, None, st3 * sgn3),
                            ]
                            S = dpt.asarray(Snp, sycl_queue=q)
                            X = S[
                                slice(None, None, st1 * sgn1),
                                slice(None, None, st2 * sgn2),
                                slice(None, None, st3 * sgn3),
                            ]
                            Y = dpt.empty((n1, n2, n3), dtype=dst_dt)
                            ti._copy_usm_ndarray_into_usm_ndarray(
                                src=X, dst=Y, queue=q
                            ).wait()
                            Ynp = _force_cast(Xnp, dst_dt)
                            assert are_close(Ynp, dpt.asnumpy(Y))
                            Yst = dpt.empty((2 * n1, n2, n3), dtype=dst_dt)[
                                ::2, ::-1
                            ]
                            ev = ti._copy_usm_ndarray_into_usm_ndarray(
                                src=X, dst=Yst, queue=q
                            )
                            Y = dpt.empty((n1, n2, n3), dtype=dst_dt)
                            ti._copy_usm_ndarray_into_usm_ndarray(
                                src=Yst, dst=Y, queue=q, depends=[ev]
                            ).wait()
                            Ynp = _force_cast(Xnp, dst_dt)
                            assert are_close(Ynp, dpt.asnumpy(Y))
