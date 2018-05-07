# CMakeLists.txt
#
# By Sebastian Raaphorst, 2003 - 2018.

set(PUBLIC_HEADER_FILES
        common.h
        bac.h
        bacoptions.h
        block.h
        blockgroup.h
        branchingscheme.h
        cliquecutproducer.h
        closestvaluebranchingscheme.h
        commandlineprocessing.h
        constraint.h
        cutproducer.h
        defaultsolutionmanager.h
        formulation.h
        generatedgroup.h
        graph.h
        group.h
        immediatesolutionmanager.h
        isomorphismcut.h
        isomorphismcutproducer.h
        lexicographicvariableorder.h
        lowestindexbranchingscheme.h
        lpsolver.h
        margotbac.h
        margotbacoptions.h
        matrix.h
        matrixgroup.h
        nibac.h
        nibac_design.h
        nibacexception.h
        problem.h
        randomvariableorder.h
        schreiersimsgroup.h
        solutionmanager.h
        statistics.h
        subsetproducer.h
        superduper.h
        timer.h
        util.h
        variableorder.h)

set(PRIVATE_HEADER_FILES
        bitstring.h
        graph.h
        node.h
        nodestack.h
        permutationpool.h
        rankedbranchingscheme.h)

set(SOURCE_FILES
        bac.cpp
        bacoptions.cpp
        bitstring.cpp
        block.cpp
        blockgroup.cpp
        branchingscheme.cpp
        cliquecutproducer.cpp
        closestvaluebranchingscheme.cpp
        commandlineprocessing.cpp
        constraint.cpp
        cutproducer.cpp
        defaultsolutionmanager.cpp
        formulation.cpp
        generatedgroup.cpp
        graph.cpp
        group.cpp
        immediatesolutionmanager.cpp
        isomorphismcut.cpp
        isomorphismcutproducer.cpp
        lexicographicvariableorder.cpp
        lowestindexbranchingscheme.cpp
        lpsolver.cpp
        margotbac.cpp
        margotbacoptions.cpp
        matrix.cpp
        matrixgroup.cpp
        nibacexception.cpp
        node.cpp
        nodestack.cpp
        permutationpool.cpp
        problem.cpp
        randomvariableorder.cpp
        rankedbranchingscheme.cpp
        schreiersimsgroup.cpp
        statistics.cpp
        subsetproducer.cpp
        superduper.cpp
        timer.cpp)

# LP-solver specific.
set(CPLEX_HEADER_FILES
        cplexsolver.h)

set(CPLEX_SOURCE_FILES
        cplexsolver.cpp)

set(GLPK_HEADER_FILES
        glpksolver.h)

set(GLPK_SOURCE_FILES
        glpksolver.cpp)



add_library(nibac SHARED
        ${SOURCE_FILES}
        ${PUBLIC_HEADER_FILES}
        ${PRIVATE_HEADER_FILES})

install(TARGETS nibac
        DESTINATION lib
        PUBLIC_HEADER DESTINATION include/nibac)
