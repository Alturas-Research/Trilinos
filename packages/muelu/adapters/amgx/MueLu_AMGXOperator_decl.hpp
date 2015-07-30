// @HEADER
//
// ***********************************************************************
//
//        MueLu: A package for multigrid based preconditioning
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef MUELU_AMGXOPERATOR_DECL_HPP
#define MUELU_AMGXOPERATOR_DECL_HPP

#if defined (HAVE_MUELU_EXPERIMENTAL) and defined (HAVE_MUELU_AMGX)
#include <Teuchos_ParameterList.hpp>

#include <Tpetra_Operator.hpp>
#include <Tpetra_CrsMatrix.hpp>
#include <Tpetra_MultiVector.hpp>
#include <Tpetra_Distributor.hpp>
#include <Tpetra_HashTable.hpp>
#include <Tpetra_Import.hpp>
#include <Tpetra_Import_Util.hpp>

#include "MueLu_Exceptions.hpp"
#include "MueLu_TpetraOperator.hpp"
#include "MueLu_VerboseObject.hpp"

#include <cuda_runtime.h>
#include <amgx_c.h>

namespace MueLu {


  /*! @class TemplatedAMGXOperator
      This templated version of the class throws errors in all methods as AmgX is not implemented for datatypes where scalar!=double/float and ordinal !=int
 */
  template <class Scalar,
            class LocalOrdinal,
            class GlobalOrdinal,
            class Node>
  class AMGXOperator : public TpetraOperator<Scalar, LocalOrdinal, GlobalOrdinal, Node> {
  private:
    typedef Scalar          SC;
    typedef LocalOrdinal    LO;
    typedef GlobalOrdinal   GO;
    typedef Node            NO;

    typedef Tpetra::Map<LO,GO,NO>            Map;
    typedef Tpetra::MultiVector<SC,LO,GO,NO> MultiVector;

  public:

    //! @name Constructor/Destructor
    //@{

    //! Constructor
    AMGXOperator(const Teuchos::RCP<Tpetra::CrsMatrix<SC,LO,GO,NO> > &InA, Teuchos::ParameterList &paramListIn) { }

    //! Destructor.
    virtual ~AMGXOperator() { }

    //@}

    //! Returns the Tpetra::Map object associated with the domain of this operator.
    Teuchos::RCP<const Map> getDomainMap() const{
      throw Exceptions::RuntimeError("Cannot use AMGXOperator with scalar != double and/or global ordinal != int \n");
    }

    //! Returns the Tpetra::Map object associated with the range of this operator.
    Teuchos::RCP<const Map> getRangeMap() const{
      throw Exceptions::RuntimeError("Cannot use AMGXOperator with scalar != double and/or global ordinal != int \n");
    }

    //! Returns a solution for the linear system AX=Y in the  Tpetra::MultiVector X.
    /*!
      \param[in]  X - Tpetra::MultiVector of dimension NumVectors that contains the solution to the linear system.
      \param[out] Y -Tpetra::MultiVector of dimension NumVectors containing the RHS of the linear system.
    */
    void apply(const MultiVector& X, MultiVector& Y, Teuchos::ETransp mode = Teuchos::NO_TRANS,
               Scalar alpha = Teuchos::ScalarTraits<Scalar>::one(), Scalar beta  = Teuchos::ScalarTraits<Scalar>::zero()) const {
      throw Exceptions::RuntimeError("Cannot use AMGXOperator with scalar != double and/or global ordinal != int \n");
    }

    //! Indicates whether this operator supports applying the adjoint operator
    bool hasTransposeApply() const{
      throw Exceptions::RuntimeError("Cannot use AMGXOperator with scalar != double and/or global ordinal != int \n");
    }

    RCP<MueLu::Hierarchy<SC,LO,GO,NO> > GetHierarchy() const {
      throw Exceptions::RuntimeError("AMGXOperator does not hold a MueLu::Hierarchy object \n");
    }

  private:
  };

  /*! @class AMGXOperator
      Creates and AmgX Solver object with a Tpetra Matrix. Partial specialization of the template for data types supported by AmgX.
  */
  template<class Node>
  class AMGXOperator<double, int, int, Node> : public TpetraOperator<double, int, int, Node> {
  private:
    typedef double  SC;
    typedef int     LO;
    typedef int     GO;
    typedef Node    NO;

    typedef Tpetra::Map<LO,GO,NO>            Map;
    typedef Tpetra::MultiVector<SC,LO,GO,NO> MultiVector;

    void printMaps(Teuchos::RCP<const Teuchos::Comm<int> >& comm, const std::vector<std::vector<int> >& vec,
                   const int* nbrs, const Map& map, const std::string& label) {
      for (int p = 0; p < comm->getSize(); p++) {
        if (comm->getRank() == p) {
          std::cout << "========\n" << label << ", lid (gid), PID  " << p << "\n========" << std::endl;

          for (size_t i = 0; i < vec.size(); ++i) {
            std::cout << "   neighbor " << nbrs[i] << " :";
            for (size_t j = 0; j < vec[i].size(); ++j)
              std::cout << " " << vec[i][j] << " (" << map.getGlobalElement(vec[i][j]) << ")";
            std::cout << std::endl;
          }
          std::cout << std::endl;
        } else {
          sleep(1);
        }
        comm->barrier();
      }
    }

  public:

    //! @name Constructor/Destructor
    //@{
    AMGXOperator(const Teuchos::RCP<Tpetra::CrsMatrix<SC,LO,GO,NO> > &inA, Teuchos::ParameterList &paramListIn) {
      RCP<const Teuchos::Comm<int> > comm = inA->getCrsGraph()->getComm();
      int myRank = comm->getRank();

      // Initialize
      AMGX_SAFE_CALL(AMGX_initialize());
      AMGX_SAFE_CALL(AMGX_initialize_plugins());

      /*system*/
      //AMGX_SAFE_CALL(AMGX_register_print_callback(&print_callback));
      AMGX_SAFE_CALL(AMGX_install_signal_handler());
      Teuchos::ParameterList configs = paramListIn.sublist("amgx:params", true);
      if (configs.isParameter("json file")) {
        AMGX_SAFE_CALL(AMGX_config_create_from_file(&Config_, (const char *) &configs.get<std::string>("json file")[0]));
      } else {
        std::ostringstream oss;
        oss << "";
        ParameterList::ConstIterator itr;
        for (itr = configs.begin(); itr != configs.end(); ++itr) {
          const std::string&    name  = configs.name(itr);
          const ParameterEntry& entry = configs.entry(itr);
          oss << name << "=" << filterValueToString(entry) << ", ";
        }
        oss << "\0";
        std::string configString = oss.str();
        if (configString == "") {
          //print msg that using defaults
          //GetOStream(Warnings0) << "Warning: No configuration parameters specified, using default AMGX configuration parameters. \n";
        }
        AMGX_SAFE_CALL(AMGX_config_create(&Config_, configString.c_str()));
      }

      // TODO: we probably need to add "exception_handling=1" to the parameter list
      // to switch on internal error handling (with no need for AMGX_SAFE_CALL)

      // NOTE: MPI communicator used in AMGX_resources_create must exist in the scope of AMGX_matrix_comm_from_maps_one_ring
      // FIXME: fix for serial comm
      RCP<const Teuchos::MpiComm<int> > tmpic = Teuchos::rcp_dynamic_cast<const Teuchos::MpiComm<int> >(comm->duplicate());
      TEUCHOS_TEST_FOR_EXCEPTION(tmpic.is_null(), Exceptions::RuntimeError, "Communicator is not MpiComm");

      RCP<const Teuchos::OpaqueWrapper<MPI_Comm> > rawMpiComm = tmpic->getRawMpiComm();
      MPI_Comm mpiComm = *rawMpiComm;

      // Construct AMGX resources
      if (comm->getSize() == 1) {
        AMGX_resources_create_simple(&Resources_, Config_);

      } else {
        int numGPUDevices;
        cudaGetDeviceCount(&numGPUDevices);
        int device[] = {(comm->getRank() % numGPUDevices)};

        AMGX_config_add_parameters(&Config_, "communicator=MPI");
        AMGX_resources_create(&Resources_, Config_, &mpiComm, 1/* number of GPU devices utilized by this rank */, device);
      }
      // std::cout << "Created Resources" << std::endl;

      AMGX_Mode mode = AMGX_mode_dDDI;
      AMGX_solver_create(&Solver_, Resources_, mode,  Config_);
      AMGX_matrix_create(&A_,      Resources_, mode);
      AMGX_vector_create(&X_,      Resources_, mode);
      AMGX_vector_create(&Y_,      Resources_, mode);

      // std::cout << "Created stuff" << std::endl;

      // Construct AMGX communication pattern
      if (comm->getSize() > 1) {
        RCP<const Tpetra::Import<LO,GO> > importer = inA->getCrsGraph()->getImporter();

        TEUCHOS_TEST_FOR_EXCEPTION(importer.is_null(), MueLu::Exceptions::RuntimeError, "The matrix A has no Import object.");

        Tpetra::Distributor distributor = importer->getDistributor();

        Array<int> sendRanks = distributor.getImagesTo();
        Array<int> recvRanks = distributor.getImagesFrom();

        std::sort(sendRanks.begin(), sendRanks.end());
        std::sort(recvRanks.begin(), recvRanks.end());

        bool match = true;
        if (sendRanks.size() != recvRanks.size()) {
          match = false;
        } else {
          for (int i = 0; i < sendRanks.size(); i++) {
            if (recvRanks[i] != sendRanks[i])
              match = false;
              break;
          }
        }
        TEUCHOS_TEST_FOR_EXCEPTION(!match, MueLu::Exceptions::RuntimeError, "AMGX requires that the processors that we send to and receive from are the same. "
                                   "This is not the case: we send to {" << sendRanks << "} and receive from {" << recvRanks << "}");

        int        num_neighbors = sendRanks.size();  // does not include the calling process
        const int* neighbors     = &sendRanks[0];

        // Later on, we'll have to organize the send and recv data by PIDs,
        // i.e, a vector V of vectors, where V[i] is PID i's vector of data.
        // Hence we need to be able to quickly look up  an array index
        // associated with each PID.
        Tpetra::Details::HashTable<int,int> hashTable(3*num_neighbors);
        for (int i = 0; i < num_neighbors; i++)
          hashTable.add(neighbors[i], i);

        // Get some information out
        ArrayView<const int> exportLIDs = importer->getExportLIDs();
        ArrayView<const int> exportPIDs = importer->getExportPIDs();
        Array<int> importPIDs;
        Tpetra::Import_Util::getPids(*importer, importPIDs, true/* make local -1 */);

        // Construct the reordering for AMGX as in AMGX_matrix_upload_all documentation
        RCP<const Map> rowMap = inA->getRowMap();
        RCP<const Map> colMap = inA->getColMap();

        int N = rowMap->getNodeNumElements(), Nc = colMap->getNodeNumElements();
        muelu2amgx_.resize(Nc, -1);

        int numUniqExports = 0;
        for (int i = 0; i < exportLIDs.size(); i++)
          if (muelu2amgx_[exportLIDs[i]] == -1) {
            numUniqExports++;
            muelu2amgx_[exportLIDs[i]] = -2;
          }

        std::cout << "[" << myRank << "]: N = " << N << ", Nc = " << Nc << std::endl;

        int localOffset = 0, exportOffset = N - numUniqExports;
        // Go through exported LIDs and put them at the end of LIDs
        for (int i = 0; i < exportLIDs.size(); i++)
          if (muelu2amgx_[exportLIDs[i]] < 0) // exportLIDs are not unique
            muelu2amgx_[exportLIDs[i]] = exportOffset++;
        // Go through all non-export LIDs, and put them at the beginning of LIDs
        for (int i = 0; i < N; i++)
          if (muelu2amgx_[i] == -1)
            muelu2amgx_[i] = localOffset++;
        // Go through the tail (imported LIDs), and order those by neighbors
        int importOffset = N;
        for (int k = 0; k < num_neighbors; k++)
          for (int i = 0; i < importPIDs.size(); i++)
            if (importPIDs[i] != -1 && hashTable.get(importPIDs[i]) == k)
              muelu2amgx_[i] = importOffset++;

        amgx2muelu_.resize(muelu2amgx_.size());
        for (int i = 0; i < muelu2amgx_.size(); i++)
          amgx2muelu_[muelu2amgx_[i]] = i;

        std::cout << "[" << comm->getRank() << "]: muelu2amgx mapping" << std::endl;
        for (int i = 0; i < Nc; i++)
          std::cout << " " << muelu2amgx_[i];
        std::cout << std::endl;

        // Construct send arrays
        std::vector<std::vector<int> > sendDatas (num_neighbors);
        std::vector<int>               send_sizes(num_neighbors, 0);
        for (int i = 0; i < exportPIDs.size(); i++) {
          int index = hashTable.get(exportPIDs[i]);
          sendDatas [index].push_back(muelu2amgx_[exportLIDs[i]]);
          send_sizes[index]++;
        }
        // FIXME: sendDatas must be sorted (based on GIDs)

        std::vector<const int*> send_maps(num_neighbors);
        for (int i = 0; i < num_neighbors; i++)
          send_maps[i] = &(sendDatas[i][0]);

        // Debugging
        printMaps(comm, sendDatas, neighbors, *importer->getTargetMap(), "send_map_vector");

        // Construct recv arrays
        std::vector<std::vector<int> > recvDatas (num_neighbors);
        std::vector<int>               recv_sizes(num_neighbors, 0);
        for (int i = 0; i < importPIDs.size(); i++)
          if (importPIDs[i] != -1) {
            int index = hashTable.get(importPIDs[i]);
            recvDatas [index].push_back(muelu2amgx_[i]);
            recv_sizes[index]++;
        }
        // FIXME: recvDatas must be sorted (based on GIDs)

        std::vector<const int*> recv_maps(num_neighbors);
        for (int i = 0; i < num_neighbors; i++)
          recv_maps[i] = &(recvDatas[i][0]);

        // Debugging
        printMaps(comm, recvDatas, neighbors, *importer->getTargetMap(), "recv_map_vector");


        // std::cout << "Created comm pattern" << std::endl;

        AMGX_SAFE_CALL(AMGX_matrix_comm_from_maps_one_ring(A_, 1, num_neighbors, neighbors, &send_sizes[0], &send_maps[0], &recv_sizes[0], &recv_maps[0]));

        // std::cout << "Uploaded matrix" << std::endl;

        AMGX_vector_bind(X_, A_);
        AMGX_vector_bind(Y_, A_);
        // std::cout << "Binded vectors" << std::endl;
      }
      // std::cout << "Created communication pattern" << std::endl;

      ArrayRCP<const size_t> ia_s;
      ArrayRCP<const int>    ja;
      ArrayRCP<const double> a;
      inA->getAllValues(ia_s, ja, a);

      ArrayRCP<int> ia(ia_s.size());
      for (int i = 0; i < ia.size(); i++)
        ia[i] = Teuchos::as<int>(ia_s[i]);

      N_      = inA->getNodeNumRows();
      int nnz = inA->getNodeNumEntries();

      // Upload matrix
      // TODO Do we need to pin memory here through AMGX_pin_memory?
      if (comm->getSize() == 1) {
        AMGX_matrix_upload_all(A_, N_, nnz, 1, 1, &ia[0], &ja[0], &a[0], NULL);

      } else {
        // Transform the matrix
        std::vector<int>    ia_new(ia.size());
        std::vector<int>    ja_new(ja.size());
        std::vector<double> a_new (a.size());

        ia_new[0] = 0;
        for (int i = 0; i < N_; i++) {
          int oldRow = amgx2muelu_[i];

          ia_new[i+1] = ia_new[i] + (ia[oldRow+1] - ia[oldRow]);

          for (int j = ia[oldRow]; j < ia[oldRow+1]; j++) {
            int offset = j - ia[oldRow];
            ja_new[ia_new[i] + offset] = muelu2amgx_[ja[j]];
            a_new [ia_new[i] + offset] = a[j];
          }
          // Do bubble sort on two arrays
          // NOTE: There are multiple possible optimizations here (even of bubble sort)
          bool swapped;
          do {
            swapped = false;

            for (int j = ia_new[i]; j < ia_new[i+1]-1; j++)
              if (ja_new[j] > ja_new[j+1]) {
                std::swap(ja_new[j], ja_new[j+1]);
                std::swap(a_new [j], a_new [j+1]);
                swapped = true;
              }
          } while (swapped == true);
        }

        std::cout << "[" << myRank << "]: ia_new" << std::endl;
        for (int i = 0; i < ia_new.size(); i++)
          std::cout << " " << ia_new[i];
        std::cout << "\n[" << myRank << "]: ja_new" << std::endl;
        for (int i = 0; i < ja_new.size(); i++)
          std::cout << " " << ja_new[i];
        std::cout << "\n[" << myRank << "]: a_new" << std::endl;
        for (int i = 0; i < a_new.size(); i++)
          std::cout << " " << a_new[i];
        std::cout << std::endl;

        AMGX_matrix_upload_all(A_, N_, nnz, 1, 1, &ia_new[0], &ja_new[0], &a_new[0], NULL);
      }
      // std::cout << "Uploaded matrix" << std::endl;

      domainMap_ = inA->getDomainMap();
      rangeMap_  = inA->getRangeMap();

      AMGX_solver_setup(Solver_, A_);
    }

    //! Destructor.
    virtual ~AMGXOperator() { }

    //! Returns the Tpetra::Map object associated with the domain of this operator.
    Teuchos::RCP<const Map> getDomainMap() const;

    //! Returns the Tpetra::Map object associated with the range of this operator.
    Teuchos::RCP<const Map> getRangeMap() const;

    //! Returns in X the solution to the linear system AX=Y.
    /*!
       \param[out] X - Tpetra::MultiVector of dimension NumVectors containing the RHS of the linear system
       \param[in]  Y - Tpetra::MultiVector of dimension NumVectors containing the solution to the linear system
       */
    void apply(const MultiVector& X, MultiVector& Y, Teuchos::ETransp mode = Teuchos::NO_TRANS,
               SC alpha = Teuchos::ScalarTraits<SC>::one(), SC beta = Teuchos::ScalarTraits<SC>::zero()) const;

    //! Indicates whether this operator supports applying the adjoint operator.
    bool hasTransposeApply() const;

    RCP<MueLu::Hierarchy<SC,LO,GO,NO> > GetHierarchy() const {
      throw Exceptions::RuntimeError("AMGXOperator does not hold a MueLu::Hierarchy object \n");
    }

    std::string filterValueToString(const Teuchos::ParameterEntry& entry ) {
      return ( entry.isList() ? std::string("...") : toString(entry.getAny()) );
    }

    int sizeA() {
      int sizeX, sizeY, n;
      AMGX_matrix_get_size(A_, &n, &sizeX, &sizeY);
      return n;
    }

    int iters() {
      int it;
      AMGX_solver_get_iterations_number(Solver_, &it);
      return it;
    }

    AMGX_SOLVE_STATUS getStatus() {
      AMGX_SOLVE_STATUS status;
      AMGX_solver_get_status(Solver_, &status);
      return status;
    }


  private:
    AMGX_solver_handle      Solver_;
    AMGX_resources_handle   Resources_;
    AMGX_config_handle      Config_;
    AMGX_matrix_handle      A_;
    AMGX_vector_handle      X_;
    AMGX_vector_handle      Y_;
    int                     N_;

    RCP<const Map>          domainMap_;
    RCP<const Map>          rangeMap_;

    std::vector<int>        muelu2amgx_;
    std::vector<int>        amgx2muelu_;
  };

} // namespace

#endif //HAVE_MUELU_EXPERIMENTAL && HAVE_MUELU_EXPERIMENTAL
#endif // MUELU_AMGXOPERATOR_DECL_HPP