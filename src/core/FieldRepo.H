#ifndef FIELDREPO_H
#define FIELDREPO_H

#include <string>
#include <unordered_map>

#include "FieldDescTypes.H"
#include "FieldUtils.H"
#include "Field.H"
#include "ScratchField.H"

#include "AMReX_AmrCore.H"
#include "AMReX_MultiFab.H"

namespace amr_wind {

/** Object that holds the MultiFab instances at all levels for a given field
 */
struct LevelDataHolder
{
    amrex::Vector<amrex::MultiFab> m_mfabs;
};

/** Field Repository
 *
 *  FieldRepo is a database that manages handling of fields used during a
 *  simulation. It provides an interface to automatically handle the
 *  creation/destruction of lower-level data structures (amrex::MultiFab) during
 *  regrids and handles the interface between the actual data and the
 *  amr_wind::Field interface which doesn't hold any actual data.
 *
 *  FieldRepo allows the user to declare instances of fields and their types
 *  (e.g., cell or node cenrered), and allows querying and accessing these
 *  fields at different places in the code and ensures the same data is accessed
 *  everywhere.
 *
 *  Fields can be declared at any time, even before the actual mesh instance is
 *  created, and it is recommended that they be created before mesh is
 *  initialized so that initialization happens automatically. When fields are
 *  declared before a mesh is available, data allocation is deferred until the
 *  mesh is generated (`MakeNewLevelFromScratch`). However, if a mesh is
 *  available, data is allocated and ready for use immediately upon declaration.
 *
 *  Field declarations use a unique string name to differentiate different
 *  fields. If amr_wind::FieldRepo::declare_field is called multiple times with
 *  the same field name, the previously created instance is returned on
 *  subsequent calls. However, an error is raised if the subsequent calls do not
 *  have the same field parameters (e.g., number of components, states, or ghost
 *  cells).
 *
 *  Fields can be accessed using amr_wind::FieldRepo::get_field method. Unlike
 *  the declaration method, this method will raise an error if no field exists.
 *  amr_wind::FieldRepo::query_field can be used to determine if a field exists
 *  in the repository.
 */
class FieldRepo
{
public:
    friend class Field;

    using FieldMap = std::unordered_map<std::string, std::unique_ptr<Field>>;

    FieldRepo(const amrex::AmrCore& mesh)
        : m_mesh(mesh)
        , m_leveldata(mesh.maxLevel() + 1)
        , m_factory(mesh.maxLevel() + 1)
    {}

    FieldRepo(const FieldRepo&) = delete;
    FieldRepo& operator=(const FieldRepo&) = delete;
    ~FieldRepo() = default;

    //! Perform field data management tasks when
    //! amrex::AmrCore::MakeNewLevelFromScratch is called
    void make_new_level_from_scratch(
        int lev, amrex::Real time,
        const amrex::BoxArray& ba,
        const amrex::DistributionMapping& dm);

    void make_new_level_from_coarse(
        int lev, amrex::Real time,
        const amrex::BoxArray& ba,
        const amrex::DistributionMapping& dm);

    void remake_level(
        int lev, amrex::Real time,
        const amrex::BoxArray& ba,
        const amrex::DistributionMapping& dm);

    void clear_level(int lev);

    /** Create a new field instance
     *
     *  @param name Unique identifier for this field
     *  @param ncomp Number of components in this field (defalt: 1)
     *  @param ngrow Number of ghost cells/nodes for this field (default: 1)
     *  @param nstates Number of time states for this field (default: 1)
     *  @param floc Field location (default: cell-centered)
     */
    Field& declare_field(
        const std::string& name,
        const int ncomp = 1,
        const int ngrow = 0,
        const int nstates = 1,
        const FieldLoc floc = FieldLoc::CELL);

    /** Declare a cell-centered field
     *
     *  This is a convenience wrapper around `declare_field`
     *
     *  \sa amr_wind::FieldRepo::declare_field
     */
    inline Field& declare_cc_field(
        const std::string& name,
        const int ncomp = 1,
        const int ngrow = 0,
        const int nstates = 1)
    {
        return declare_field(name, ncomp, ngrow, nstates, FieldLoc::CELL);
    }

    /** Declare a node-centered field
     *
     *  This is a convenience wrapper around `declare_field`
     *
     *  \sa amr_wind::FieldRepo::declare_field
     */
    inline Field& declare_nd_field(
        const std::string& name,
        const int ncomp = 1,
        const int ngrow = 0,
        const int nstates = 1)
    {
        return declare_field(name, ncomp, ngrow, nstates, FieldLoc::NODE);
    }

    /** Declare a face field on the faces with normal in x-direction
     *
     *  This is a convenience wrapper around `declare_field`
     *
     *  \sa amr_wind::FieldRepo::declare_field
     */
    inline Field& declare_xf_field(
        const std::string& name,
        const int ncomp = 1,
        const int ngrow = 0,
        const int nstates = 1)
    {
        return declare_field(name, ncomp, ngrow, nstates, FieldLoc::XFACE);
    }

    /** Declare a face field on the faces with normal in y-direction
     *
     *  This is a convenience wrapper around `declare_field`
     *
     *  \sa amr_wind::FieldRepo::declare_field
     */
    inline Field& declare_yf_field(
        const std::string& name,
        const int ncomp = 1,
        const int ngrow = 0,
        const int nstates = 1)
    {
        return declare_field(name, ncomp, ngrow, nstates, FieldLoc::YFACE);
    }

    /** Declare a face field on the faces with normal in z-direction
     *
     *  This is a convenience wrapper around `declare_field`
     *
     *  \sa amr_wind::FieldRepo::declare_field
     */
    inline Field& declare_zf_field(
        const std::string& name,
        const int ncomp = 1,
        const int ngrow = 0,
        const int nstates = 1)
    {
        return declare_field(name, ncomp, ngrow, nstates, FieldLoc::ZFACE);
    }

    /** Declare a vector field with components in the x,y,z faces
     *
     *  This is a convenience function around `declare_field`
     */
    inline amrex::Vector<Field*> declare_face_normal_field(
        const amrex::Vector<std::string>& names,
        const int ncomp = 1,
        const int ngrow = 0,
        const int nstates = 1)
    {
        AMREX_ASSERT(names.size() == AMREX_SPACEDIM);
        return {
            &declare_field(names[0], ncomp, ngrow, nstates, FieldLoc::XFACE),
            &declare_field(names[1], ncomp, ngrow, nstates, FieldLoc::YFACE),
            &declare_field(names[2], ncomp, ngrow, nstates, FieldLoc::ZFACE),
        };
    }

    /** Return a previously created field identified by name and time state
     */
    Field& get_field(
        const std::string& name, const FieldState fstate = FieldState::New) const;

    //! Query if field uniquely identified by name and time state exists in repository
    bool field_exists(
        const std::string& name, const FieldState fstate = FieldState::New) const;

    std::unique_ptr<ScratchField> create_scratch_field(
        const std::string& name,
        const int ncomp = 1,
        const int ngrow = 0,
        const FieldLoc floc = FieldLoc::CELL) const;

    std::unique_ptr<ScratchField> create_scratch_field(
        const int ncomp = 1,
        const int ngrow = 0,
        const FieldLoc floc = FieldLoc::CELL) const;

    //! Advance all fields with more than one timestate to the new timestep
    void advance_states() noexcept;

    const amrex::AmrCore& mesh() const { return m_mesh; }

    //! Total number of levels currently active in the AMR mesh
    int num_active_levels() const noexcept { return m_mesh.finestLevel() + 1; }

    //! Number of fields registered in the database
    int num_fields() const noexcept { return m_field_vec.size(); }

    //! Return list of fields registered (for unit-test purposes only)
    const amrex::Vector<std::unique_ptr<Field>>& fields() const { return m_field_vec; }

    //! Return factory instance at a given level
    inline const amrex::FabFactory<amrex::FArrayBox>&
    factory(int lev) const noexcept { return *m_factory[lev]; }

protected:
    /** Return the amrex::MultiFab instance for a field at a given level
     *
     *  \param fid Unique integer field identifier for this field
     *  \param lev AMR level
     */
    inline amrex::MultiFab& get_multifab(const unsigned fid, const int lev) noexcept
    {
        BL_ASSERT(lev <= m_mesh.finestLevel());
        return m_leveldata[lev]->m_mfabs[fid];
    }

    Field& create_state(Field& field, const FieldState fstate) noexcept;

    //! Allocate field data for a single level outside of regrid
    void allocate_field_data(
        int lev,
        Field& field,
        LevelDataHolder& fdata,
        const amrex::FabFactory<amrex::FArrayBox>& factory);

    //! Allocate field data at all levels
    void allocate_field_data(Field& field);

    //! Allocate data at a level during regrid
    void allocate_field_data(
        const amrex::BoxArray& ba,
        const amrex::DistributionMapping& dm,
        LevelDataHolder& fdata,
        const amrex::FabFactory<amrex::FArrayBox>& factory);

    //! Reference to the mesh instance
    const amrex::AmrCore& m_mesh;

    //! Array (size: nlevels) of data holder that contains another array of
    //! MultiFabs for all fields at that level.
    amrex::Vector<std::unique_ptr<LevelDataHolder>> m_leveldata;

    //! Array of Factory objects at all levels
    amrex::Vector<std::unique_ptr<amrex::FabFactory<amrex::FArrayBox>>> m_factory;

    //! References to field instances identified by unique integer
    mutable amrex::Vector<std::unique_ptr<Field>> m_field_vec;

    //! Map of field name to unique integer ID for lookups
    std::unordered_map<std::string, size_t> m_fid_map;

    //! Flag indicating if mesh is available to allocate field data
    bool m_is_initialized{false};
};

} // namespace amr_wind

#endif /* FIELDREPO_H */
