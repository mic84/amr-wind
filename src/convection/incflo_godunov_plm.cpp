#include <incflo_convection_K.H>
#include "Godunov.H"

using namespace amrex;

void godunov::predict_plm (int lev, Box const& bx, int ncomp,
                          Array4<Real> const& Imx, Array4<Real> const& Ipx,
                          Array4<Real> const& Imy, Array4<Real> const& Ipy,
                          Array4<Real> const& Imz, Array4<Real> const& Ipz,
                          Array4<Real const> const& q,
                          Array4<Real const> const& vcc,
                          Vector<Geometry> geom,
                          Real dt,
                          amrex::Gpu::DeviceVector<amrex::BCRec>& bcrec_device)
{
    const Real dx = geom[lev].CellSize(0);
    const Real dy = geom[lev].CellSize(1);
    const Real dz = geom[lev].CellSize(2);
    Real dtdx = dt/dx;
    Real dtdy = dt/dy;
    Real dtdz = dt/dz;

    const Box& domain_box = geom[lev].Domain();
    const int domain_ilo = domain_box.smallEnd(0);
    const int domain_ihi = domain_box.bigEnd(0);
    const int domain_jlo = domain_box.smallEnd(1);
    const int domain_jhi = domain_box.bigEnd(1);
    const int domain_klo = domain_box.smallEnd(2);
    const int domain_khi = domain_box.bigEnd(2);

    // At an ext_dir boundary, the boundary value is on the face, not cell center.

    BCRec const* pbc = bcrec_device.data();

    if ((domain_ilo >= bx.smallEnd(0)-1) or (domain_ihi <= bx.bigEnd(0)))
    {
        amrex::ParallelFor(bx, ncomp, [q,vcc,domain_ilo,domain_ihi,Imx,Ipx,dtdx,pbc]
        AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
        {
            const auto& bc = pbc[n];
            bool extdir_ilo = bc.lo(0) == BCType::ext_dir;
            bool extdir_ihi = bc.hi(0) == BCType::ext_dir;

            Real upls = q(i  ,j,k,n) + 0.5 * (-1.0 - vcc(i  ,j,k,0) * dtdx) * 
                incflo_ho_xslope_extdir(i,j,k,n,q, extdir_ilo, extdir_ihi, domain_ilo, domain_ihi);
            Real umns = q(i-1,j,k,n) + 0.5 * ( 1.0 - vcc(i-1,j,k,0) * dtdx) * 
                incflo_ho_xslope_extdir(i-1,j,k,n,q, extdir_ilo, extdir_ihi, domain_ilo, domain_ihi);

            if (extdir_ilo and i == domain_ilo) {
                umns = q(i-1,j,k,n);
                upls = q(i-1,j,k,n);
            } else if (extdir_ihi and i == domain_ihi+1) {
                umns = q(i,j,k,n);
                upls = q(i,j,k,n);
            }

            Ipx(i-1,j,k,n) = umns;
            Imx(i  ,j,k,n) = upls;
        });
    }
    else
    {
        amrex::ParallelFor(bx, ncomp, [q,vcc,Ipx,Imx,dtdx]
        AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
        {
            Real upls = q(i  ,j,k,n) + 0.5 * (-1.0 - vcc(i  ,j,k,0) * dtdx) * 
                incflo_ho_xslope(i  ,j,k,n,q);
            Real umns = q(i-1,j,k,n) + 0.5 * ( 1.0 - vcc(i-1,j,k,0) * dtdx) * 
                incflo_ho_xslope(i-1,j,k,n,q);

            Ipx(i-1,j,k,n) = umns;
            Imx(i  ,j,k,n) = upls;
        });
    }

    if ((domain_jlo >= bx.smallEnd(1)-1) or (domain_jhi <= bx.bigEnd(1)))
    {
        amrex::ParallelFor(bx, ncomp, [q,vcc,domain_jlo,domain_jhi,Imy,Ipy,dtdy,pbc]
        AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
        {
            const auto& bc = pbc[n];
            bool extdir_jlo = bc.lo(0) == BCType::ext_dir;
            bool extdir_jhi = bc.hi(0) == BCType::ext_dir;

            Real vpls = q(i,j  ,k,n) + 0.5 * (-1.0 - vcc(i,j  ,k,1) * dtdy) *
                incflo_ho_yslope_extdir(i,j,k,n,q, extdir_jlo, extdir_jhi, domain_jlo, domain_jhi);
            Real vmns = q(i,j-1,k,n) + 0.5 * ( 1.0 - vcc(i,j-1,k,1) * dtdy) * 
                incflo_ho_yslope_extdir(i,j-1,k,n,q, extdir_jlo, extdir_jhi, domain_jlo, domain_jhi);

            if (extdir_jlo and j == domain_jlo) {
                vmns = q(i,j-1,k,n);
                vpls = q(i,j-1,k,n);
            } else if (extdir_jhi and j == domain_jhi+1) {
                vmns = q(i,j,k,n);
                vpls = q(i,j,k,n);
            }

            Ipy(i,j-1,k,n) = vmns;
            Imy(i,j  ,k,n) = vpls;
        });
    }
    else
    {
        amrex::ParallelFor(bx, ncomp, [q,vcc,Ipy,Imy,dtdy]
        AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
        {
            Real vpls = q(i,j  ,k,n) + 0.5 * (-1.0 - vcc(i,j  ,k,1) * dtdy) * 
                incflo_ho_yslope(i,j  ,k,n,q);
            Real vmns = q(i,j-1,k,n) + 0.5 * ( 1.0 - vcc(i,j-1,k,1) * dtdy) * 
                incflo_ho_yslope(i,j-1,k,n,q);

            Ipy(i,j-1,k,n) = vmns;
            Imy(i,j  ,k,n) = vpls;
        });
    }

    if ((domain_klo >= bx.smallEnd(2)-1) or (domain_khi <= bx.bigEnd(2)))
    {
        amrex::ParallelFor(bx, ncomp, [q,vcc,domain_klo,domain_khi,Ipz,Imz,dtdz,pbc]
        AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
        {
            const auto& bc = pbc[n];
            bool extdir_klo = bc.lo(0) == BCType::ext_dir;
            bool extdir_khi = bc.hi(0) == BCType::ext_dir;

            Real wpls = q(i,j,k  ,n) + 0.5 * (-1.0 - vcc(i,j,k  ,2) * dtdz) * 
                incflo_ho_zslope_extdir(i,j,k,n,q, extdir_klo, extdir_khi, domain_klo, domain_khi);
            Real wmns = q(i,j,k-1,n) + 0.5 * ( 1.0 - vcc(i,j,k-1,2) * dtdz) * 
                incflo_ho_zslope_extdir(i,j,k-1,n,q, extdir_klo, extdir_khi, domain_klo, domain_khi);

            if (extdir_klo and k == domain_klo) {
                wmns = vcc(i,j,k-1,n);
                wpls = vcc(i,j,k-1,n);
            } else if (extdir_khi and k == domain_khi+1) {
                wmns = vcc(i,j,k,n);
                wpls = vcc(i,j,k,n);
            }

            Ipz(i,j,k-1,n) = wmns;
            Imz(i,j,k  ,n) = wpls;
        });
    }
    else
    {
        amrex::ParallelFor(bx, ncomp, [q,vcc,Ipz,Imz,dtdz]
        AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
        {
            Real wpls = q(i,j,k  ,n) + 0.5 * (-1.0 - vcc(i,j,k  ,2) * dtdz) * 
                incflo_ho_zslope(i,j,k  ,n,q);
            Real wmns = q(i,j,k-1,n) + 0.5 * ( 1.0 - vcc(i,j,k-1,2) * dtdz) * 
                incflo_ho_zslope(i,j,k-1,n,q);

            Ipz(i,j,k-1,n) = wmns;
            Imz(i,j,k  ,n) = wpls;
        });
    }
}

