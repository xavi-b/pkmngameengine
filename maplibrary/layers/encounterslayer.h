#ifndef ENCOUNTERSLAYER_H
#define ENCOUNTERSLAYER_H

#include <vector>
#include <memory>
#include "layer.h"

enum EncounterType
{
    NONE,
    GRASS,
    WATER
};

void          tag_invoke(js::value_from_tag, js::value& jv, EncounterType const& o);
EncounterType tag_invoke(js::value_to_tag<EncounterType>, js::value const& jv);

class EncountersLayer : public Layer<EncounterType>
{
public:
    EncountersLayer(size_t nCol, size_t nRow);

    friend void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<EncountersLayer> const& o);
    friend std::unique_ptr<EncountersLayer> tag_invoke(js::value_to_tag<std::unique_ptr<EncountersLayer>>,
                                                       js::value const& jv);
};

#endif // ENCOUNTERSLAYER_H
