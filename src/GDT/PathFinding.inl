template <typename T, typename IndexType>
std::unordered_map<IndexType, IndexType> GDT::findPath(
    const T* array,
    const IndexType& start,
    const IndexType& end,
    const IndexType& width,
    const IndexType& size,
    const std::function<bool(const T*, const IndexType&)>& isObstacle
)
{
    std::unordered_map<IndexType, IndexType> cameFrom;
    std::unordered_map<IndexType, IndexType> costSoFar;
    std::priority_queue<GDT::Internal::FindPathData<IndexType> > frontier;

    frontier.push(GDT::Internal::FindPathData<IndexType>(end, 0));
    cameFrom.insert(std::make_pair(end, end));
    costSoFar.insert(std::make_pair(end, 0));

    while(!frontier.empty())
    {
        GDT::Internal::FindPathData<IndexType> current = frontier.top();
        frontier.pop();

        if(current.index == start)
        {
            break;
        }

        GDT::Internal::FindPathData<IndexType> next;
        IndexType nextCost = costSoFar.at(current.index) + 1;
        for(unsigned int i = 0; i < 4; ++i)
        {
            switch(i)
            {
            case 0: // left
                if(current.index % width == 0)
                {
                    continue;
                }
                next.index = current.index - 1;
                break;
            case 1: // right
                if(current.index % width == width - 1)
                {
                    continue;
                }
                next.index = current.index + 1;
                break;
            case 2: // up
                if(current.index / width == 0)
                {
                    continue;
                }
                next.index = current.index - width;
                break;
            case 3: // down
                if(current.index / width == size / width - 1)
                {
                    continue;
                }
                next.index = current.index + width;
                break;
            }

            if(!isObstacle(array, next.index)
                && (costSoFar.find(next.index) == costSoFar.end()
                    || nextCost < costSoFar.at(next.index)))
            {
                costSoFar.erase(next.index);
                costSoFar.insert(std::make_pair(next.index, nextCost));
                next.priority = nextCost + GDT::Internal::heuristic(
                    width, next.index, start);
                frontier.push(next);
                cameFrom.insert(std::make_pair(next.index, current.index));
            }
        }
    }

    return cameFrom;
}
