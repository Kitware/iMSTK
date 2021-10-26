%inline %{
#include <atomic>
class DebugAndTestOnly {
public:
    static constexpr int NULL_PTR = -2;
    static constexpr int CAST_FAILURE = -1;

    static int use_count(void* objPtr, const std::string& typeName)
    {
        if (objPtr == nullptr) return NULL_PTR;

        if ("Geometry" == typeName)
        {
            std::shared_ptr<imstk::Geometry>* obj = (std::shared_ptr<imstk::Geometry>*) objPtr; 
            return obj->use_count();
        }
        else if ("PointSet" == typeName)
        {
            std::shared_ptr<imstk::PointSet>* obj = (std::shared_ptr<imstk::PointSet>*) objPtr; 
            return obj->use_count();
        }
        else if ("SurfaceMesh" == typeName)
        {
            std::shared_ptr<imstk::SurfaceMesh>* obj = (std::shared_ptr<imstk::SurfaceMesh>*) objPtr; 
            return obj->use_count();
        }
        else if ("TetrahedralMesh" == typeName)
        {
            std::shared_ptr<imstk::TetrahedralMesh>* obj = (std::shared_ptr<imstk::TetrahedralMesh>*) objPtr; 
            return obj->use_count();
        }

        return CAST_FAILURE;
    }

    DebugAndTestOnly()
    {
        printf("DebugAndTestOnly: constructed\n");
        m_numCreated += 1; 
    }

    ~DebugAndTestOnly()
    {
        printf("DebugAndTestOnly: destructed\n");
        m_numDestroyed += 1;
    }

    static int getNumCreated()
    {
        return m_numCreated;
    }

    static int getNumDestroyed()
    {
        return m_numDestroyed;
    }
private:
    static std::atomic<int> m_numCreated;
    static std::atomic<int> m_numDestroyed;
};

std::atomic<int> DebugAndTestOnly::m_numCreated{0};
std::atomic<int> DebugAndTestOnly::m_numDestroyed{0};

%}

%shared_ptr(DebugAndTestOnly)
