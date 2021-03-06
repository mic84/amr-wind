#include "density/density.H"

namespace amr_wind {
namespace pde {

template class PDESystem<Density, fvm::Godunov>;
template class PDESystem<Density, fvm::MOL>;

} // namespace pde
} // namespace amr_wind
