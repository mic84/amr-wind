//
//  Godunov.H
//  amr-wind
//

#ifndef Godunov_H
#define Godunov_H

#include "FieldRepo.H"

namespace godunov {

void predict_godunov(
    amr_wind::FieldRepo& repo,
    const amr_wind::FieldState fstate,
    amrex::Real dt,
    bool godunov_ppm,
    bool godunov_use_forces_in_trans);

void compute_convective_term(
    amr_wind::FieldRepo& repo,
    const amr_wind::FieldState fstate,
    amrex::Real dt,
    bool constant_density,
    bool advect_tracer,
    bool godunov_ppm);

void compute_advection(
    int lev,
    amrex::Box const& bx,
    int ncomp,
    amrex::Array4<amrex::Real> const& dqdt,
    amrex::Array4<amrex::Real const> const& q,
    amrex::Array4<amrex::Real const> const& umac,
    amrex::Array4<amrex::Real const> const& vmac,
    amrex::Array4<amrex::Real const> const& wmac,
    amrex::Array4<amrex::Real const> const& fq,
    amrex::BCRec const* pbc,
    int const* iconserv,
    amrex::Real* p,
    amrex::Vector<amrex::Geometry> geom,
    amrex::Real dt,
    bool godunov_ppm);

void predict_ppm(
    int lev,
    amrex::Box const& bx,
    int ncomp,
    amrex::Array4<amrex::Real> const& Imx,
    amrex::Array4<amrex::Real> const& Ipx,
    amrex::Array4<amrex::Real> const& Imy,
    amrex::Array4<amrex::Real> const& Ipy,
    amrex::Array4<amrex::Real> const& Imz,
    amrex::Array4<amrex::Real> const& Ipz,
    amrex::Array4<amrex::Real const> const& q,
    amrex::Array4<amrex::Real const> const& vel,
    amrex::Vector<amrex::Geometry> geom,
    amrex::Real dt,
    amrex::Gpu::DeviceVector<amrex::BCRec>& bcrec_device);

void predict_plm(
    int lev,
    amrex::Box const& bx,
    int ncomp,
    amrex::Array4<amrex::Real> const& Imx,
    amrex::Array4<amrex::Real> const& Ipx,
    amrex::Array4<amrex::Real> const& Imy,
    amrex::Array4<amrex::Real> const& Ipy,
    amrex::Array4<amrex::Real> const& Imz,
    amrex::Array4<amrex::Real> const& Ipz,
    amrex::Array4<amrex::Real const> const& q,
    amrex::Array4<amrex::Real const> const& vcc,
    amrex::Vector<amrex::Geometry> geom,
    amrex::Real dt,
    amrex::Gpu::DeviceVector<amrex::BCRec>& bcrec_device);

void make_trans_velocities(
    int lev,
    amrex::Box const& xbx,
    amrex::Box const& ybx,
    amrex::Box const& zbx,
    amrex::Array4<amrex::Real> const& u_ad,
    amrex::Array4<amrex::Real> const& v_ad,
    amrex::Array4<amrex::Real> const& w_ad,
    amrex::Array4<amrex::Real const> const& Imx,
    amrex::Array4<amrex::Real const> const& Ipx,
    amrex::Array4<amrex::Real const> const& Imy,
    amrex::Array4<amrex::Real const> const& Ipy,
    amrex::Array4<amrex::Real const> const& Imz,
    amrex::Array4<amrex::Real const> const& Ipz,
    amrex::Array4<amrex::Real const> const& vel,
    amrex::Array4<amrex::Real const> const& f,
    amrex::Vector<amrex::Geometry> geom,
    amrex::Real dt,
    amrex::Gpu::DeviceVector<amrex::BCRec>& bcrec_device,
    bool godunov_use_forces_in_trans);

void predict_godunov(
    int lev,
    amrex::Box const& bx,
    int ncomp,
    amrex::Box const& xbx,
    amrex::Box const& ybx,
    amrex::Box const& zbx,
    amrex::Array4<amrex::Real> const& qx,
    amrex::Array4<amrex::Real> const& qy,
    amrex::Array4<amrex::Real> const& qz,
    amrex::Array4<amrex::Real const> const& q,
    amrex::Array4<amrex::Real const> const& u_ad,
    amrex::Array4<amrex::Real const> const& v_ad,
    amrex::Array4<amrex::Real const> const& w_ad,
    amrex::Array4<amrex::Real> const& Imx,
    amrex::Array4<amrex::Real> const& Ipx,
    amrex::Array4<amrex::Real> const& Imy,
    amrex::Array4<amrex::Real> const& Ipy,
    amrex::Array4<amrex::Real> const& Imz,
    amrex::Array4<amrex::Real> const& Ipz,
    amrex::Array4<amrex::Real const> const& f,
    amrex::Real* p,
    amrex::Vector<amrex::Geometry> geom,
    amrex::Real dt,
    amrex::Gpu::DeviceVector<amrex::BCRec>& bcrec_device,
    bool godunov_use_forces_in_trans);

} // namespace godunov

#endif /* Godunov_H */
