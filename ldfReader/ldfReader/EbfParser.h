#ifndef EbfParser_H
#define EbfParser_H 1

/** @class EbfParser
@brief Provides access to the EBF parsing routines and is the gateway to
filling the LatData structure.

$Header$
*/

namespace ldfReader {
    class EbfParser {
    public:

        EbfParser() {};

        virtual ~EbfParser() { };

        virtual void clear() = 0;

        /// Load data for the current event in the EBF file
        virtual int loadData() = 0;

        /// Moves event pointer to the next event in the EBF file
        virtual int nextEvent() = 0;

        /// Turn on or off debug output.  Returns old value of flag
        virtual bool setDebug(bool on);

        // local exception class
        class Exception{ };

    private:

    };
}
#endif
