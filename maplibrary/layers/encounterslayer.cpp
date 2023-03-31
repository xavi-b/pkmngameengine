#include "encounterslayer.h"

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, EncounterType const& o)
{
    jv = static_cast<int>(o);
}

EncounterType tag_invoke(js::value_to_tag<EncounterType>, boost::json::value const& jv)
{
    return static_cast<EncounterType>(js::value_to<int>(jv));
}

EncountersLayer::EncountersLayer(int nCol, int nRow) : Layer<EncounterType>(nCol, nRow)
{
}

void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<EncountersLayer> const& o)
{
    jv = {{"elements", js::value_from<std::vector<std::vector<EncounterType>> const&>(o->elements)},
          {"nCol", o->nCol},
          {"nRow", o->nRow}};
}

std::unique_ptr<EncountersLayer> tag_invoke(js::value_to_tag<std::unique_ptr<EncountersLayer>>, js::value const& jv)
{
    js::object const&                obj = jv.as_object();
    std::unique_ptr<EncountersLayer> o =
        std::make_unique<EncountersLayer>(js::value_to<int>(obj.at("nCol")), js::value_to<int>(obj.at("nRow")));

    o->elements = js::value_to<std::vector<std::vector<EncounterType>>>(obj.at("elements"));

    return o;
}
