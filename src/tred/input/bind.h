#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include "tred/input/range.h"


namespace input
{


struct Table;


struct Node
{
    virtual ~Node() = default;

    virtual float GetValue() = 0;
};


struct InputNode : public Node
{
    InputNode();

    float state;

    float GetValue() override;
};


struct NodeDef
{
    virtual ~NodeDef() = default;

    virtual std::unique_ptr<Node> Create() = 0;
};


struct Output
{
    Output(int node, const std::string& var, Range range);

    int node;
    std::string var;
    Range range;
};


struct ConverterDef
{
    void AddOutput(const std::string& name, const std::string& var, Range range);
    int AddVar(const std::string& name, std::unique_ptr<NodeDef>&& Node);

    int GetNode(const std::string& name);

    std::vector<std::unique_ptr<NodeDef>> vars;
    std::map<std::string, int> nodes;
    std::vector<Output> outputs;
};


struct Converter
{
    explicit Converter(const ConverterDef& converter);
    
    void SetRawState(int var, float value);
    void SetTable(Table* table);

    std::vector<std::unique_ptr<Node>> vars;
    std::vector<Output> outputs;
};


struct ValueReciever
{
    ValueReciever(Table* table, Converter* converter);

    Table* table;
    Converter* converter;
};


template<typename T>
struct BindDef
{
    template<typename TT> BindDef(const std::string& v, const T& t, const TT& tt, ConverterDef* converter)
        : node(converter->GetNode(v))
        , type(t)
        , invert(tt.invert)
        , scale(tt.scale)
    {
    }

    int node;
    T type;
    bool invert;
    float scale;
};


template<typename T>
struct BindMap
{
    BindMap(const std::vector<BindDef<T>>& src, Converter* c)
        : converter(c)
    {
        for(const auto& b: src)
        {
            using M = decltype(binds);
            using P = typename M::value_type;
            binds.insert(P{b.type, b.node});
        }
    }

    void Recieve(ValueReciever*)
    {
    }

    void SetRaw(const T& t, float state)
    {
        auto found = binds.find(t);
        if (found != binds.end())
        {
            converter->SetRawState(found->second, state);
        }
    }

    Converter* converter;
    std::map<T, int> binds;
};

}
