template <typename IndexType>
IndexType GDT::Internal::heuristic(IndexType width, IndexType a, IndexType b)
{
    return std::abs(a % width - b % width) + std::abs(a / width - b / width);
}
template <typename IndexType>
GDT::Internal::FindPathData<
        IndexType,
        typename std::enable_if<std::is_integral<IndexType>::value>::type
    >::FindPathData() :
index(0),
priority(0)
{}

template <typename IndexType>
GDT::Internal::FindPathData<
        IndexType,
        typename std::enable_if<std::is_integral<IndexType>::value>::type
    >::FindPathData(
            IndexType i,
            IndexType priority) :
index(i),
priority(priority)
{}

template <typename IndexType>
bool GDT::Internal::FindPathData<
        IndexType,
        typename std::enable_if<std::is_integral<IndexType>::value>::type
    >::operator < (const FindPathData<IndexType>& other) const
{
    return other.priority < priority;
}
