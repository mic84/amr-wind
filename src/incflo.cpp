
#include <incflo.H>

#include "ABL.H"
#include "RefinementCriteria.H"
#include "PDE.H"

using namespace amrex;

incflo::incflo ()
    : m_repo(*this)
{
    // NOTE: Geometry on all levels has just been defined in the AmrCore
    // constructor. No valid BoxArray and DistributionMapping have been defined.
    // But the arrays for them have been resized.

    m_time.parse_parameters();
    // Read inputs file using ParmParse
    ReadParameters();

    declare_fields();

    init_field_bcs();

    set_background_pressure();
}

incflo::~incflo ()
{}

void incflo::InitData ()
{
    BL_PROFILE("incflo::InitData()");

    int restart_flag = 0;
    if(m_restart_file.empty())
    {
        // This tells the AmrMesh class not to iterate when creating the initial
        // grid hierarchy
        // SetIterateToFalse();

        // This tells the Cluster routine to use the new chopping routine which
        // rejects cuts if they don't improve the efficiency
        SetUseNewChop();

        // This is an AmrCore member function which recursively makes new levels
        // with MakeNewLevelFromScratch.
        InitFromScratch(m_time.current_time());

        if (m_do_initial_proj) {
            InitialProjection();
        }
        if (m_initial_iterations > 0) {
            InitialIterations();
        }

        // xxxxx TODO averagedown ???

        if (m_time.write_checkpoint()) { WriteCheckPointFile(); }
    }
    else
    {
        restart_flag = 1;
        // Read starting configuration from chk file.
        ReadCheckpointFile();
    }

    // Plot initial distribution
    if(m_time.write_plot_file() && !restart_flag)
    {
        WritePlotFile();
        m_last_plt = 0;
    }
    if(m_KE_int > 0 && !restart_flag)
    {
        amrex::Print() << "Time, Kinetic Energy: " << m_time.current_time() << ", " << ComputeKineticEnergy() << std::endl;
    }


    if (m_verbose > 0 and ParallelDescriptor::IOProcessor()) {
        printGridSummary(amrex::OutStream(), 0, finest_level);
    }
}

void incflo::Evolve()
{
    BL_PROFILE("incflo::Evolve()");

    while(m_time.new_timestep())
    {
        if (m_time.do_regrid())
        {
            if (m_verbose > 0) amrex::Print() << "Regriding...\n";
            regrid(0, m_time.current_time());
            if (m_verbose > 0 and ParallelDescriptor::IOProcessor()) {
                printGridSummary(amrex::OutStream(), 0, finest_level);
            }
        }

        // Advance to time t + dt
        Advance();

        if (m_time.write_plot_file())
        {
            WritePlotFile();
            m_last_plt = m_time.time_index();
        }

        if(m_time.write_checkpoint())
        {
            WriteCheckPointFile();
            m_last_chk = m_time.time_index();
        }
        
        if(m_KE_int > 0 && (m_time.time_index() % m_KE_int == 0))
        {
            amrex::Print() << "Time, Kinetic Energy: " << m_time.current_time() << ", " << ComputeKineticEnergy() << std::endl;
        }
    }

    // TODO: Fix last checkpoint/plot output
    // Output at the final time
    if( m_time.write_last_checkpoint()) {
        WriteCheckPointFile();
    }
    if( m_time.write_last_plot_file())
    {
        WritePlotFile();
    }
}

// Make a new level from scratch using provided BoxArray and DistributionMapping.
// Only used during initialization.
// overrides the pure virtual function in AmrCore
void incflo::MakeNewLevelFromScratch (int lev, Real time, const BoxArray& new_grids,
                                      const DistributionMapping& new_dmap)
{
    BL_PROFILE("incflo::MakeNewLevelFromScratch()");

    if (m_verbose > 0)
    {
        amrex::Print() << "Making new level " << lev << " from scratch" << std::endl;
        if (m_verbose > 2) {
            amrex::Print() << "with BoxArray " << new_grids << std::endl;
        }
    }

    SetBoxArray(lev, new_grids);
    SetDistributionMap(lev, new_dmap);

    m_repo.make_new_level_from_scratch(lev, time, new_grids, new_dmap);

    if (m_restart_file.empty()) {
        prob_init_fluid(lev);

        for (auto& pp: m_physics) {
            pp->initialize_fields(lev, Geom(lev));
        }
    }
}

// Set covered coarse cells to be the average of overlying fine cells
// TODO: Move somewhere else, for example setup/incflo_arrays.cpp
void incflo::AverageDown()
{
    BL_PROFILE("incflo::AverageDown()");

    for (int lev = finest_level - 1; lev >= 0; --lev)
    {
        AverageDownTo(lev);
    }
}

void incflo::AverageDownTo(int /* crse_lev */)
{
    amrex::Abort("xxxxx TODO AverageDownTo");
}
