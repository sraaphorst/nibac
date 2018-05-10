/**
 * commandlineprocessing.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef COMMANDLINEPROCESSING_H
#define COMMANDLINEPROCESSING_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace vorpal::nibac {
// Class forwards
    class BACOptions;

    class BranchingScheme;

    class BranchingSchemeCreator;

    class CutProducer;

    class CutProducerCreator;

    class MargotBACOptions;

    class SolutionManager;

    class SolutionManagerCreator;

    class VariableOrder;

    class VariableOrderCreator;

    /**
    * This is a simple, standardized command-line interface that can be used to configure either
    * an instance of BACOptions or MargotBACOptions, saving the user the trouble of doing so.
    */
    class CommandLineProcessing final {
    public:
        // Return status if help is requested.
        static const int HELP;

    protected:
        BACOptions &options;

        std::map<int, CutProducerCreator* > cutProducerCreators;
        std::map<CutProducerCreator* , bool> cutProducerCreatorDefaults;
        std::vector<CutProducer* > cutProducers;

        std::map<int, SolutionManagerCreator* > solutionManagerCreators;
        int defaultSolutionManagerCreatorID;
        SolutionManagerCreator *solutionManagerCreator;
        SolutionManagerCreator *defaultSolutionManagerCreator;
        SolutionManager *solutionManager;

        std::map<int, VariableOrderCreator* > variableOrderCreators;
        int defaultVariableOrderCreatorID;
        VariableOrderCreator *variableOrderCreator;
        VariableOrderCreator *defaultVariableOrderCreator;
        VariableOrder *variableOrder;

        std::map<int, BranchingSchemeCreator* > branchingSchemeCreators;
        int defaultBranchingSchemeCreatorID;
        BranchingSchemeCreator *branchingSchemeCreator;
        BranchingSchemeCreator *defaultBranchingSchemeCreator;
        BranchingScheme *branchingScheme;

    public:
        CommandLineProcessing(BACOptions &);

        virtual ~CommandLineProcessing();

        // Before populating the options, the coder can register permissible cut producer creators
        // with this class. This allows the user to choose which cuts, amongst those offered, will
        // be used, and whether or not they will be active by default.
        void registerCreator(CutProducerCreator &, int, bool);

        // Similarly, we register solution managers.
        // The one registered with true will be used as the default.
        void registerCreator(SolutionManagerCreator &, int, bool);

        // Similarly for variable orders.
        void registerCreator(VariableOrderCreator &, int, bool);

        // Similarly for branching schemes.
        void registerCreator(BranchingSchemeCreator &, int, bool);

        // After everything has been registered, we can process the command line arguments based on
        // what structure we wish to have populated.
        int populateBACOptions(int &, char **&);

        int populateMargotBACOptions(int &, char **&);

        // When all the necessary information has been sent to the cut producer creators,
        // etc, we create the objects we need.
        void finishBACOptionsConfiguration(void);

        void finishMargotBACOptionsConfiguration(void);

        // We can also print the possible command line options to a stream.
        void outputOptions(std::ostream &);

    protected:
        // Convenience method to print an optionsMap in a standardized way.
        static void outputOptionsMap(std::ostream &,
                                     const std::map <std::string, std::pair<std::string, std::string>> &);
    };
};
#endif
