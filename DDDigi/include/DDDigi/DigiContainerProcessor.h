//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================
#ifndef DDDIGI_DIGICONTAINERPROCESSOR_H
#define DDDIGI_DIGICONTAINERPROCESSOR_H

// Framework include files
#include <DD4hep/Callback.h>
#include <DDDigi/DigiData.h>
#include <DDDigi/DigiEventAction.h>
#include <DDDigi/DigiParallelWorker.h>

/// C/C++ include files
#include <set>

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {

  /// Namespace for the Digitization part of the AIDA detector description toolkit
  namespace digi {

    /// Forward declarations
    class DigiSegmentContext;
    class DigiContainerSequence;
    class DigiContainerProcessor;
    class DigiContainerSequenceAction;
    class DigiMultiContainerProcessor;
    
    /// Worker base class to analyse containers from the input segment in parallel
    /**
     *
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_DIGITIZATION
     */
    class DigiContainerProcessor : public DigiAction   {
    public:
      /// Structure imports
      using action_t       = DigiAction;
      using segment_t      = DataSegment;
      using property_t     = PropertyManager;
      using segmentation_t = DigiSegmentContext;

      /// Input definition
      struct input_t  {
	/// Input data key
	Key             key;
	/// Input deposits
	std::any&       data;
      };

      /// Output handle definition
      struct output_t  {
	int             mask;
	segment_t&      data;
      };

      /// Hit processing predicate
      struct predicate_t  {
	Callback callback                   { };
	uint32_t id                         { 0 };
	const segmentation_t* segmentation  { nullptr };
	predicate_t() = default;
	predicate_t(predicate_t&& copy) = default;
	predicate_t(const predicate_t& copy) = default;
	predicate_t(const Callback& c, uint32_t i, const segmentation_t* s)
	  : callback(c), id(i), segmentation(s) {}
	predicate_t& operator = (predicate_t&& copy) = default;
	predicate_t& operator = (const predicate_t& copy) = default;
	/// Check if a deposit should be processed
	bool operator()(const std::pair<const CellID, EnergyDeposit>& deposit)   const;
      };

      /// Work definition
      struct work_t  {
	/// Event processing context
	context_t&        context;
	/// Input data
	input_t           input;
	/// Output data
	output_t&         output;
	/// Optional properties
	const property_t& properties;

	/// Basic check if input data are present
	bool has_input()  const    {  return this->input.data.has_value();  }
	/// Access key of input data
	Key  input_key()  const    {  return this->input.key;               }
	/// Access the input data type
	const std::type_info& input_type()  const;
	/// String form of the input data type
	std::string input_type_name()  const;
	/// Access input data by type
	template <typename DATA> DATA* get_input(bool exc=false);
	/// Access input data by type
	template <typename DATA> const DATA* get_input(bool exc=false)  const;
      };

    protected:
      /// Define standard assignments and constructors
      DDDIGI_DEFINE_ACTION_CONSTRUCTORS(DigiContainerProcessor);

    public:
      /// Access to default callback
      static const predicate_t& accept_all();

      /// Standard constructor
      DigiContainerProcessor(const kernel_t& kernel, const std::string& name);
      /// Default destructor
      virtual ~DigiContainerProcessor();
      /// Main functional callback adapter
      virtual void execute(context_t& context, work_t& work, const predicate_t& predicate)  const;
    };

    /// Check if a deposit should be processed
    inline bool DigiContainerProcessor::predicate_t::operator()(const std::pair<const CellID, EnergyDeposit>& deposit)   const   {
      const void* args[] = { &deposit };
      return this->callback.execute(args);
    }

    /// Worker class act on containers in an event identified by input masks and container name
    /**
     *  The sequencer calls all registered processors for the contaiers registered.
     *
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_DIGITIZATION
     */
    class DigiContainerSequence : public DigiContainerProcessor  {
    protected:
      /// Structure imports
      using self_t      = DigiContainerSequence;
      using processor_t = DigiContainerProcessor;
      using worker_t    = DigiParallelWorker<processor_t,work_t>;
      using workers_t   = DigiParallelWorkers<worker_t>;
      friend class DigiParallelWorker<processor_t,work_t>;

    protected:
      /**  Member variables                           */
      /// Property to steer parallel processing
      bool               m_parallel { false };
      /// Array of sub-workers
      workers_t          m_workers;
      /// Lock for output merging
      mutable std::mutex m_output_lock;

    protected:
      /// Define standard assignments and constructors
      DDDIGI_DEFINE_ACTION_CONSTRUCTORS(DigiContainerSequence);
      /// Default destructor
      virtual ~DigiContainerSequence();

      /// Get hold of the registered processor for a given container
      worker_t* need_registered_worker(Key item_key)  const;

    public:
      /// Standard constructor
      DigiContainerSequence(const kernel_t& kernel, const std::string& name);
      /// Adopt new parallel worker
      virtual void adopt_processor(DigiContainerProcessor* action);
      /// Main functional callback adapter
      virtual void execute(context_t& context, work_t& work, const predicate_t& predicate)  const;
    };

    /// Worker base class to analyse containers from the input segment in parallel
    /**
     *  Depending on the adopted processors, the full input record is scanned and
     *  the registered processors are called.
     *
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_DIGITIZATION
     */
    class DigiContainerSequenceAction : public DigiEventAction  {

    protected:
      /// Structure imports
      using action_t    = DigiAction;
      using self_t      = DigiContainerSequenceAction;
      using processor_t = DigiContainerProcessor;
      using output_t    = processor_t::output_t;
      using property_t  = processor_t::property_t;

      /// Single worker work item definition
      struct work_item_t  {
	Key key;
	std::any* data;
      };
      /// Work definition structure
      /// Argument structure for client calls
      struct work_t  {
	context_t&             context;
	std::vector<work_item_t> input_items;
	output_t&                output;
	const property_t&        properties;
	const self_t&            parent;
      };

      using worker_t         = DigiParallelWorker<processor_t, work_t>;
      using workers_t        = DigiParallelWorkers<worker_t>;
      using reg_workers_t    = std::map<Key, worker_t*>;
      using reg_processors_t = std::map<Key, processor_t*>;
      friend class DigiParallelWorker<processor_t, work_t>;

      /// Array of sub-workers
      workers_t          m_workers;
      /// Registered action map
      reg_processors_t   m_registered_processors;
      /// Registered worker map
      reg_workers_t      m_registered_workers;

      /// Property: Input data segment name
      std::string        m_input_segment  { "inputs" };
      /// Property: Input mask to be handled
      int                m_input_mask     { 0x0 };
      /// Property: Input data segment name
      std::string        m_output_segment { "outputs" };
      /// Property: event mask for output data
      int                m_output_mask    { 0x0 };

      /// Lock for output merging
      mutable std::mutex m_output_lock;
      
    protected:
      /// Define standard assignments and constructors
      DDDIGI_DEFINE_ACTION_CONSTRUCTORS(DigiContainerSequenceAction);

      /// Default destructor
      virtual ~DigiContainerSequenceAction();
      /// Initialization callback
      virtual void initialize();

      /// Get hold of the registered processor for a given container
      worker_t* need_registered_worker(Key item_key, bool exc=true)  const;

    public:
      /// Standard constructor
      DigiContainerSequenceAction(const kernel_t& kernel, const std::string& name);
      /// Adopt new parallel worker acting on one single container
      void adopt_processor(DigiContainerProcessor* action, const std::string& container);
      /// Adopt new parallel worker acting on multiple containers
      void adopt_processor(DigiContainerProcessor* action, const std::vector<std::string>& containers);
      /// Main functional callback if specific work is known
      virtual void execute(context_t& context)  const override;
    };

    /// Sequencer class to analyse containers from the input segment in parallel
    /**
     *
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_DIGITIZATION
     */
    class DigiMultiContainerProcessor : virtual public DigiEventAction   {
    protected:
      using self_t        = DigiMultiContainerProcessor;
      using processor_t   = DigiContainerProcessor;
      using worker_keys_t = std::vector<std::vector<Key> >;
      using work_items_t  = std::vector<std::pair<Key, std::any*> >;
      using output_t      = processor_t::output_t;
      using property_t    = processor_t::property_t;

      /// Argument structure required to support multiple client calls
      struct work_t  {
	context_t&        context;
	work_items_t&     items;
	output_t&         output;
	const property_t& properties;
	const self_t&     parent;
      };
      using worker_t      = DigiParallelWorker<processor_t, work_t>;
      using workers_t     = DigiParallelWorkers<worker_t>;
      friend class DigiParallelWorker<processor_t, work_t>;

    protected:
      /// Property: Input data segment name
      std::string        m_input_segment { "inputs" };
      /// Property: event masks to be handled
      std::vector<int>   m_input_masks  { };
      /// Property: Input data segment name
      std::string        m_output_segment { "outputs" };
      /// Property: event mask for output data
      int                m_output_mask  { 0x0 };

      /// Set of container names to be used by this processor
      std::map<std::string, std::vector<processor_t*> >    m_processors;
      std::map<Key::itemkey_type, std::vector<worker_t*> > m_worker_map;

      /// Set of work items to be processed and passed to clients
      std::set<Key>             m_work_items;
      /// Set of keys required by each worker
      worker_keys_t             m_worker_keys;
      /// Ordered list of actions registered
      std::vector<processor_t*> m_actions;
      /// Lock for output merging
      mutable std::mutex        m_output_lock;

      /// Array of sub-workers
      workers_t          m_workers;

    protected:
       /// Define standard assignments and constructors
      DDDIGI_DEFINE_ACTION_CONSTRUCTORS(DigiMultiContainerProcessor);
      /// Default destructor
      virtual ~DigiMultiContainerProcessor();
      /// Initialize action object
      void initialize();

    public:
      /// Standard constructor
      DigiMultiContainerProcessor(const kernel_t& kernel, const std::string& name);
      const std::vector<Key>& worker_keys(size_t worker_id)  const  {
	return this->m_worker_keys.at(worker_id);
      }
      const std::vector<int>& input_masks()  const   {
	return this->m_input_masks;
      }
      /// Adopt new parallel worker
      void adopt_processor(DigiContainerProcessor* action, const std::vector<std::string>& containers);
      /// Main functional callback
      virtual void execute(context_t& context)  const;
    };
  }    // End namespace digi
}      // End namespace dd4hep
#endif // DDDIGI_DIGICONTAINERPROCESSOR_H
