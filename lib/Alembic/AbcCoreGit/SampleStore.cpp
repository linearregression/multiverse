//-*****************************************************************************
//
// Copyright (c) 2014,
//
// All rights reserved.
//
//-*****************************************************************************

#include <Alembic/AbcCoreGit/SampleStore.h>
#include <Alembic/AbcCoreGit/Utils.h>

namespace Alembic {
namespace AbcCoreGit {
namespace ALEMBIC_VERSION_NS {

template <typename T>
void TRACE_VALUE(std::string msg, T v)
{
    TRACE(msg << "UNKNOWN TYPE");
}

template <>
void TRACE_VALUE(std::string msg, Util::bool_t v)
{
    TRACE(msg << " (u::bool_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::uint8_t v)
{
    TRACE(msg << " (u::uint8_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::int8_t v)
{
    TRACE(msg << " (u::int8_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::uint16_t v)
{
    TRACE(msg << " (u::uint16_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::int16_t v)
{
    TRACE(msg << " (u::int16_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::uint32_t v)
{
    TRACE(msg << " (u::uint32_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::int32_t v)
{
    TRACE(msg << " (u::int32_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::uint64_t v)
{
    TRACE(msg << " (u::uint64_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::int64_t v)
{
    TRACE(msg << " (u::int64_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::float16_t v)
{
    TRACE(msg << " (u::float16_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::float32_t v)
{
    TRACE(msg << " (u::float32_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::float64_t v)
{
    TRACE(msg << " (u::float64_t) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::string v)
{
    TRACE(msg << " (u::string) " << v);
}

template <>
void TRACE_VALUE(std::string msg, Util::wstring v)
{
    TRACE(msg << " (u::wstring) (DON'T KNOW HOW TO REPRESENT)");
}

template <>
void TRACE_VALUE(std::string msg, char v)
{
    TRACE(msg << " (char) " << v);
}

template <typename T>
TypedSampleStore<T>::TypedSampleStore( const AbcA::DataType &iDataType, const AbcA::Dimensions &iDims )
    : m_dataType(iDataType)
    , m_dimensions(iDims)
{
}

template <typename T>
TypedSampleStore<T>::TypedSampleStore( const void *iData, const AbcA::DataType &iDataType, const AbcA::Dimensions &iDims )
    : m_dataType(iDataType)
    , m_dimensions(iDims)
{
    copyFrom( iData );
}

template <typename T>
TypedSampleStore<T>::~TypedSampleStore()
{
}

template <typename T>
void TypedSampleStore<T>::copyFrom( const std::vector<T>& iData )
{
    m_data = iData;
}

template <typename T>
void TypedSampleStore<T>::copyFrom( const T* iData )
{
    const T *iDataT = reinterpret_cast<const T *>( iData );
    size_t N = m_data.size();
    for ( size_t i = 0; i < N; ++i )
    {
        m_data[i] = iDataT[i];
    }
}

template <typename T>
void TypedSampleStore<T>::copyFrom( const void* iData )
{
    const T *iDataT = reinterpret_cast<const T *>( iData );
    copyFrom( iDataT );
}

template <typename T>
int TypedSampleStore<T>::sampleIndexToDataIndex( int sampleIndex )
{
    if (rank() == 0)
    {
        size_t extent = m_dataType.getExtent();

        return sampleIndex * extent;
    } else
    {
        assert( rank() >= 1 );

        size_t extent = m_dataType.getExtent();
        size_t points_per_sample = m_dimensions.numPoints();
        size_t pods_per_sample = points_per_sample * extent;

        return sampleIndex * pods_per_sample;
    }
}

template <typename T>
int TypedSampleStore<T>::dataIndexToSampleIndex( int dataIndex )
{
    if (dataIndex == 0)
        return 0;

    if (rank() == 0)
    {
        size_t extent = m_dataType.getExtent();

        return dataIndex / extent;
    } else
    {
        assert( rank() >= 1 );

        size_t extent = m_dataType.getExtent();
        size_t points_per_sample = m_dimensions.numPoints();
        size_t pods_per_sample = points_per_sample * extent;

        return dataIndex / pods_per_sample;
    }
}

template <typename T>
void TypedSampleStore<T>::getSamplePieceT( T* iIntoLocation, size_t dataIndex, int index, int subIndex )
{
    Alembic::Util::PlainOldDataType curPod = m_dataType.getPod();

    TRACE( "TypedSampleStore::getSamplePiece(dataIndex:" << dataIndex << " index:" << index << " subIndex:" << subIndex << ")   #bytes:" << PODNumBytes( curPod ) );
    ABCA_ASSERT( (curPod != Alembic::Util::kStringPOD) && (curPod != Alembic::Util::kWstringPOD),
        "Can't convert " << m_dataType <<
        "to non-std::string / non-std::wstring" );

    *iIntoLocation = m_data[dataIndex];
}

template <typename T>
void TypedSampleStore<T>::getSampleT( T* iIntoLocation, int index )
{
    Alembic::Util::PlainOldDataType curPod = m_dataType.getPod();

    TRACE( "TypedSampleStore::getSample() index:" << index << "  #bytes:" << PODNumBytes( curPod ) );
    ABCA_ASSERT( (curPod != Alembic::Util::kStringPOD) && (curPod != Alembic::Util::kWstringPOD),
        "Can't convert " << m_dataType <<
        "to non-std::string / non-std::wstring" );

    if (rank() == 0)
    {
        size_t extent = m_dataType.getExtent();
        size_t baseIndex = (index * extent);
        for (size_t i = 0; i < extent; ++i)
            getSamplePieceT( iIntoLocation + i, baseIndex + i, index, i );
    } else
    {
        assert( rank() >= 1 );

        size_t extent = m_dataType.getExtent();
        size_t points_per_sample = m_dimensions.numPoints();
        size_t pods_per_sample = points_per_sample * extent;
        size_t baseIndex = (index * pods_per_sample);

        for (size_t i = 0; i < pods_per_sample; ++i)
            getSamplePieceT( iIntoLocation + i, baseIndex + i, index, i );
    }
}

template <>
void TypedSampleStore<std::string>::getSampleT( std::string* iIntoLocation, int index )
{
    Alembic::Util::PlainOldDataType curPod = m_dataType.getPod();

    TODO("FIXME!!!");

    ABCA_ASSERT( curPod == Alembic::Util::kStringPOD,
        "Can't convert " << m_dataType <<
        "to std::string" );

    std::string src = m_data[index];
    std::string * strPtr =
        reinterpret_cast< std::string * > ( iIntoLocation );

    size_t numChars = src.size() + 1;
    char * buf = new char[ numChars + 1 ];
    std::copy(src.begin(), src.end(), buf);
    buf[src.size()] = '\0';

    std::size_t startStr = 0;
    std::size_t strPos = 0;

    for ( std::size_t i = 0; i < numChars; ++i )
    {
        if ( buf[i] == 0 )
        {
            strPtr[strPos] = buf + startStr;
            startStr = i + 1;
            strPos ++;
        }
    }

    delete [] buf;
}

template <typename T>
void TypedSampleStore<T>::getSample( void *iIntoLocation, int index )
{
    T* iIntoLocationT = reinterpret_cast<T*>(iIntoLocation);
    getSampleT(iIntoLocationT, index);
}

template <typename T>
void TypedSampleStore<T>::getSample( AbcA::ArraySamplePtr& oSample, int index )
{
    // how much space do we need?
    Util::Dimensions dims = getDimensions();

    oSample = AbcA::AllocateArraySample( getDataType(), dims );

    ABCA_ASSERT( oSample->getDataType() == m_dataType,
        "DataType on ArraySample oSamp: " << oSample->getDataType() <<
        ", does not match the DataType of the SampleStore: " <<
        m_dataType );

    getSample( const_cast<void*>( oSample->getData() ), index );

    // AbcA::ArraySample::Key key = oSample->getKey();
}

// type traits
template <typename T> struct scalar_traits
{
    static T Zero() { return static_cast<T>(0); }
};

template <>
struct scalar_traits<bool>
{
    static bool Zero() { return false; }
};

template <>
struct scalar_traits<Util::bool_t>
{
    static Util::bool_t Zero() { return false; }
};

template <>
struct scalar_traits<float>
{
    static float Zero() { return 0.0; }
};

template <>
struct scalar_traits<double>
{
    static double Zero() { return 0.0; }
};

template <>
struct scalar_traits<std::string>
{
    static std::string Zero() { return ""; }
};

template <typename T>
void TypedSampleStore<T>::addSample( const T* iSamp, const AbcA::ArraySample::Key& key )
{
    std::string key_str = key.digest.str();

    TRACE("SampleStore::addSample(key:'" << key_str << "' #bytes:" << key.numBytes << ")");

    size_t at = m_data.size();
    if (! m_key_pos.count(key_str))
        m_key_pos[key_str] = std::vector<size_t>();
    m_key_pos[key_str].push_back( at );
    m_pos_key[at] = key;

    if (rank() == 0)
    {
        size_t extent = m_dataType.getExtent();

        if (! iSamp)
        {
            for (size_t i = 0; i < extent; ++i)
                addSamplePiece( scalar_traits<T>::Zero() );
        } else
        {
            for (size_t i = 0; i < extent; ++i)
                addSamplePiece( iSamp[i] );
        }
    } else
    {
        assert( rank() >= 1 );

        size_t extent = m_dataType.getExtent();
        size_t points_per_sample = m_dimensions.numPoints();
        size_t pods_per_sample = points_per_sample * extent;

        if (! iSamp)
        {
            for (size_t i = 0; i < pods_per_sample; ++i)
                addSamplePiece( scalar_traits<T>::Zero() );
        } else
        {
            for (size_t i = 0; i < pods_per_sample; ++i)
                addSamplePiece( iSamp[i] );
        }
    }
}

template <typename T>
void TypedSampleStore<T>::addSample( const void *iSamp, const AbcA::ArraySample::Key& key )
{
    const T* iSampT = reinterpret_cast<const T*>(iSamp);
    addSample(iSampT, key);
}

template <typename T>
void TypedSampleStore<T>::addSample( const AbcA::ArraySample& iSamp )
{
    ABCA_ASSERT( iSamp.getDataType() == m_dataType,
        "DataType on ArraySample iSamp: " << iSamp.getDataType() <<
        ", does not match the DataType of the SampleStore: " <<
        m_dataType );

    AbcA::ArraySample::Key key = iSamp.getKey();

    addSample( iSamp.getData(), key );
}

template <typename T>
void TypedSampleStore<T>::setFromPreviousSample()                           // duplicate last added sample
{
    ABCA_ASSERT( m_data.size() > 0,
        "No samples to duplicate in SampleStore" );

    if (rank() == 0)
    {
        size_t extent = m_dataType.getExtent();

        ABCA_ASSERT( m_data.size() > extent,
                      "wrong number of PODs in SampleStore" );

        std::vector<T> last_sample(m_data.end() - extent, m_data.end());

        assert(last_sample.size() == extent);
        ABCA_ASSERT( last_sample.size() == extent,
                      "wrong number of PODs in last sample" );

        m_data.insert( m_data.end(), last_sample.begin(), last_sample.end() );
    } else
    {
        assert( rank() >= 1 );

        size_t extent = m_dataType.getExtent();
        size_t points_per_sample = m_dimensions.numPoints();
        size_t pods_per_sample = points_per_sample * extent;

        ABCA_ASSERT( m_data.size() > pods_per_sample,
                      "wrong number of PODs in SampleStore" );

        std::vector<T> last_sample(m_data.end() - pods_per_sample, m_data.end());

        assert(last_sample.size() == pods_per_sample);
        ABCA_ASSERT( last_sample.size() == pods_per_sample,
                      "wrong number of PODs in last sample" );

        m_data.insert( m_data.end(), last_sample.begin(), last_sample.end() );
    }
}

template <typename T>
bool TypedSampleStore<T>::getKey( AbcA::index_t iSampleIndex, AbcA::ArraySampleKey& oKey )
{
    int dataIndex = sampleIndexToDataIndex( iSampleIndex );

    if (m_pos_key.count(dataIndex))
    {
        oKey = m_pos_key[dataIndex];
        return true;
    }

    return false;
}

template <typename T>
size_t TypedSampleStore<T>::getNumSamples() const
{
    if (rank() == 0)
    {
        size_t data_count = m_data.size();

        if (data_count == 0)
            return 0;

        ABCA_ASSERT( (data_count % m_dataType.getExtent()) == 0,
                      "wrong number of PODs in SampleStore" );
        return data_count / m_dataType.getExtent();
    } else
    {
        assert( rank() >= 1 );

        size_t points_per_sample = m_dimensions.numPoints();
        size_t pods_per_sample = points_per_sample * m_dataType.getExtent();

        size_t data_count = m_data.size();

        if (data_count == 0)
            return 0;

        ABCA_ASSERT( (data_count % pods_per_sample) == 0,
                      "wrong number of PODs in SampleStore" );
        return data_count / pods_per_sample;
    }
}

template <typename T>
std::string TypedSampleStore<T>::getFullTypeName() const
{
    std::ostringstream ss;
    ss << m_dataType;
    return ss.str();
}

template <typename T>
std::string TypedSampleStore<T>::repr(bool extended) const
{
    std::ostringstream ss;
    if (extended)
    {
        ss << "<TypedSampleStore samples:" << getNumSamples() <<  " type:" << m_dataType << " dims:" << m_dimensions << ">";
    } else
    {
        ss << "<TypedSampleStore samples:" << getNumSamples() << ">";
    }
    return ss.str();
}


/*
 * ScalarProperty: # of samples is fixed
 * ArrayProperty: # of samples is variable
 *
 * dimensions: array of integers (eg: [], [ 1 ], [ 6 ])
 * rank:       dimensions.rank() == len(dimensions) (eg: 0, 1, ...)
 * extent:     number of scalar values in basic element
 *
 * example: dimensions = [2, 2], rank = 3, type = f
 * corresponds to a 2x2 matrix of vectors made of 3 ing point values (3f)
 *
 * ScalarProperty has rank 0 and ArrayProperty rank > 0 ?
 *
 * # points per sample: dimensions.numPoints() == product of all dimensions' values
 *
 */

class JsonArrayBuffer
{
public:
    JsonArrayBuffer(size_t dim) :
        m_arr(Json::arrayValue), m_el(Json::arrayValue), m_dim(dim), m_cnt(0) {}
    virtual ~JsonArrayBuffer() {}

    operator Json::Value() { return json(); }

    Json::Value json()
    {
        _flush();
        Json::Value r = m_arr;
        clear();
        return r;
    }

    JsonArrayBuffer& clear()
    {
        m_arr = Json::Value( Json::arrayValue );
        m_el = Json::Value( Json::arrayValue );
        m_cnt = 0;
        return *this;
    }

    template <typename V>
    JsonArrayBuffer& push( const V& value )
    {
        if (m_dim <= 0)
        {
            m_arr.append( TypedSampleStore<V>::JsonFromValue(value) );
            return *this;
        }

        m_el.append( TypedSampleStore<V>::JsonFromValue(value) );
        m_cnt++;
        if (m_cnt >= m_dim)
        {
            m_arr.append( m_el );
            m_el = Json::Value( Json::arrayValue );
            m_cnt = 0;
        }

        return *this;
    }

private:
    JsonArrayBuffer& _flush()
    {
        if (m_cnt >= 1)
        {
            m_arr.append( m_el );
            m_el = Json::Value( Json::arrayValue );
            m_cnt = 0;
        }
        return *this;
    }

    Json::Value m_arr;
    Json::Value m_el;
    size_t      m_dim;
    size_t      m_cnt;
};

std::string pod2str(const Alembic::Util::PlainOldDataType& pod)
{
    std::ostringstream ss;
    ss << PODName( pod );
    return ss.str();
}

template <typename T>
Json::Value TypedSampleStore<T>::json() const
{
    Json::Value root( Json::objectValue );

    {
        std::ostringstream ss;
        ss << PODName( m_dataType.getPod() );
        root["typename"] = ss.str();
    }

    {
        std::ostringstream ss;
        ss << m_dataType;
        root["type"] = ss.str();
    }

    assert( m_dataType.getExtent() == extent() );

    root["extent"] = extent();
    root["rank"] = TypedSampleStore<size_t>::JsonFromValue( rank() );

    root["num_samples"] = TypedSampleStore<size_t>::JsonFromValue( getNumSamples() );

    root["dimensions"] = TypedSampleStore<AbcA::Dimensions>::JsonFromValue( getDimensions() );

    {
        Json::Value pos_k( Json::arrayValue );

        std::map<size_t, AbcA::ArraySample::Key>::const_iterator p_it;
        for (p_it = m_pos_key.begin(); p_it != m_pos_key.end(); ++p_it)
        {
            size_t at = (*p_it).first;
            const AbcA::ArraySample::Key& key = (*p_it).second;

            Json::Value el( Json::arrayValue );
            el.append( TypedSampleStore<size_t>::JsonFromValue( at ) );

            Json::Value j_key( Json::objectValue );
            j_key["numBytes"] = TypedSampleStore<size_t>::JsonFromValue( key.numBytes );
            j_key["origPOD"] = pod2str(key.origPOD);
            j_key["readPOD"] = pod2str(key.readPOD);
            j_key["digest"] = key.digest.str();

            el.append( j_key );
            pos_k.append( el );
        }

        root["keys"] = pos_k;
    }

    Json::Value data( Json::arrayValue );

    size_t extent = m_dataType.getExtent();

    if (extent == 1)
    {
        typename std::vector<T>::const_iterator it;
        for (it = m_data.begin(); it != m_data.end(); ++it)
        {
            data.append( JsonFromValue(*it) );
        }
    } else
    {
        ABCA_ASSERT( extent > 1, "wrong extent" );

        JsonArrayBuffer e_ab( extent );

        typename std::vector<T>::const_iterator it;
        for (it = m_data.begin(); it != m_data.end(); ++it)
        {
            e_ab.push( (*it) );
        }
        data = e_ab;
    }

    root["data"] = data;

    return root;
}

template <typename T>
Json::Value TypedSampleStore<T>::JsonFromValue( const T& iValue )
{
    Json::Value data = iValue;
    return data;
}

template <typename T>
T TypedSampleStore<T>::ValueFromJson( const Json::Value& jsonValue )
{
    T value = jsonValue;
    return value;
}

template <>
Json::Value TypedSampleStore<AbcA::Dimensions>::JsonFromValue( const AbcA::Dimensions& iDims )
{
    Json::Value data( Json::arrayValue );
    for ( size_t i = 0; i < iDims.rank(); ++i )
        data.append( static_cast<Json::Value::UInt64>( iDims[i] ) );
    return data;
}

template <>
Json::Value TypedSampleStore<Util::bool_t>::JsonFromValue( const Util::bool_t& iValue )
{
    Json::Value data = iValue.asBool();
    return data;
}

template <>
Util::bool_t TypedSampleStore<Util::bool_t>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<Util::bool_t>(jsonValue.asBool());
}

template <>
Json::Value TypedSampleStore<half>::JsonFromValue( const half& iValue )
{
    Json::Value data = static_cast<double>(iValue);
    return data;
}

template <>
half TypedSampleStore<half>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<half>(jsonValue.asDouble());
}

template <>
float TypedSampleStore<float>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<float>(jsonValue.asFloat());
}

template <>
double TypedSampleStore<double>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<double>(jsonValue.asDouble());
}

template <>
Json::Value TypedSampleStore<long>::JsonFromValue( const long& iValue )
{
    Json::Value data = static_cast<Json::Int64>(iValue);
    return data;
}

template <>
long TypedSampleStore<long>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<long>(jsonValue.asInt64());
}

template <>
int TypedSampleStore<int>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<int>(jsonValue.asInt());
}

template <>
Json::Value TypedSampleStore<long unsigned>::JsonFromValue( const long unsigned& iValue )
{
    Json::Value data = static_cast<Json::UInt64>(iValue);
    return data;
}

template <>
long unsigned TypedSampleStore<long unsigned>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<long unsigned>(jsonValue.asUInt64());
}

template <>
unsigned int TypedSampleStore<unsigned int>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<unsigned int>(jsonValue.asUInt());
}

template <>
short TypedSampleStore<short>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<short>(jsonValue.asInt());
}

template <>
unsigned short TypedSampleStore<unsigned short>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<unsigned short>(jsonValue.asUInt());
}

template <>
Json::Value TypedSampleStore<Util::wstring>::JsonFromValue( const Util::wstring& iValue )
{
    std::string s((const char*)&iValue[0], sizeof(wchar_t)/sizeof(char)*iValue.size());
    Json::Value data = s;
    return data;
}

template <>
Util::wstring TypedSampleStore<Util::wstring>::ValueFromJson( const Json::Value& jsonValue )
{
    std::wstringstream wss;
    wss << jsonValue.asString().c_str();
    return wss.str();
}

template <>
Util::string TypedSampleStore<Util::string>::ValueFromJson( const Json::Value& jsonValue )
{
    return jsonValue.asString();
}

template <>
Json::Value TypedSampleStore< wchar_t >::JsonFromValue( const wchar_t& iValue )
{
    Json::Value data = static_cast<int>(iValue);
    return data;
}

template <>
wchar_t TypedSampleStore<wchar_t>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<wchar_t>(jsonValue.asInt());
}

template <>
signed char TypedSampleStore<signed char>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<signed char>(jsonValue.asInt());
}

template <>
unsigned char TypedSampleStore<unsigned char>::ValueFromJson( const Json::Value& jsonValue )
{
    return static_cast<unsigned char>(jsonValue.asUInt());
}

uint8_t hexchar2int(char input)
{
    if ((input >= '0') && (input <= '9'))
        return (input - '0');
    if ((input >= 'A') && (input <= 'F'))
        return (input - 'A') + 10;
    if ((input >= 'a') && (input <= 'f'))
        return (input - 'a') + 10;
    ABCA_THROW( "invalid input char" );
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
size_t hex2bin(uint8_t *dst, const char* src)
{
    uint8_t *dst_orig = dst;

    while (*src && src[1])
    {
        *dst      = hexchar2int(*(src++)) << 4;
        *(dst++) |= hexchar2int(*(src++));
    }

    return (dst - dst_orig);
}

template <typename T>
void TypedSampleStore<T>::fromJson(const Json::Value& root)
{
    TRACE("TypedSampleStore<T>::fromJson()");

    std::string v_typename = root.get("typename", "UNKNOWN").asString();
    std::string v_type = root.get("type", "UNKNOWN").asString();

    uint8_t v_extent = root.get("extent", 0).asUInt();
    size_t v_rank = root.get("rank", 0).asUInt();
    size_t v_num_samples = root.get("num_samples", 0).asUInt();

    Json::Value v_dimensions = root["dimensions"];
    Json::Value v_data = root["data"];

    AbcA::Dimensions dimensions;
    dimensions.setRank( v_dimensions.size() );
    {
        int idx = 0;
        for (Json::Value::iterator it = v_dimensions.begin(); it != v_dimensions.end(); ++it)
        {
            dimensions[idx] = (*it).asUInt64();
            idx++;
        }
    }

    ABCA_ASSERT( dimensions.rank() == v_rank, "wrong dimensions rank" );

    Alembic::Util::PlainOldDataType pod = Alembic::Util::PODFromName( v_typename );
    AbcA::DataType dataType(pod, v_extent);

    ABCA_ASSERT( dataType.getExtent() == v_extent, "wrong datatype extent" );

    m_dataType = dataType;
    m_dimensions = dimensions;

    ABCA_ASSERT( rank() == v_rank, "wrong rank" );
    ABCA_ASSERT( extent() == v_extent, "wrong extent" );

    m_data.clear();

    for (Json::Value::iterator it = v_data.begin(); it != v_data.end(); ++it)
    {
        if ((*it).isArray())
        {
            Json::Value& sub = *it;
            for (Json::Value::iterator sub_it = sub.begin(); sub_it != sub.end(); ++sub_it)
            {
                m_data.push_back( ValueFromJson(*sub_it) );
            }
        } else
        {
            m_data.push_back( ValueFromJson(*it) );
        }
    }

    ABCA_ASSERT( getNumSamples() == v_num_samples, "wrong number of samples" );

    // read back keys

    Json::Value v_keys = root["keys"];

    for (Json::Value::iterator it = v_keys.begin(); it != v_keys.end(); ++it)
    {
        ABCA_ASSERT( (*it).isArray(), "wrong kind of 'keys' element" );

        if ((*it).isArray())
        {
            Json::Value& sub = *it;
            Json::Value j_pos = sub[0];
            Json::Value j_key = sub[1];

            AbcA::ArraySample::Key key;

            size_t at = static_cast<size_t>(j_pos.asUInt64());

            std::string key_str = j_key["digest"].asString();

            key.numBytes = static_cast<size_t>(j_key["numBytes"].asUInt64());
            key.origPOD = Alembic::Util::PODFromName( j_key["origPOD"].asString() );
            key.readPOD = Alembic::Util::PODFromName( j_key["readPOD"].asString() );
            hex2bin(key.digest.d, key_str.c_str());

            //std::string key_str = j_key.asString();

            if (! m_key_pos.count(key_str))
                m_key_pos[key_str] = std::vector<size_t>();
            m_key_pos[key_str].push_back( at );
            m_pos_key[at] = key;
        }
    }
}

AbstractTypedSampleStore* BuildSampleStore( const AbcA::DataType &iDataType, const AbcA::Dimensions &iDims )
{
    size_t extent = iDataType.getExtent();

    TRACE("BuildSampleStore(iDataType pod:" << PODName( iDataType.getPod() ) << " extent:" << extent << ")");
    ABCA_ASSERT( iDataType.getPod() != Alembic::Util::kUnknownPOD && (extent > 0),
                 "Degenerate data type" );
    switch ( iDataType.getPod() )
    {
    case Alembic::Util::kBooleanPOD:
        return new TypedSampleStore<Util::bool_t>( iDataType, iDims ); break;
    case Alembic::Util::kUint8POD:
        return new TypedSampleStore<Util::uint8_t>( iDataType, iDims ); break;
    case Alembic::Util::kInt8POD:
        return new TypedSampleStore<Util::int8_t>( iDataType, iDims ); break;
    case Alembic::Util::kUint16POD:
        return new TypedSampleStore<Util::uint16_t>( iDataType, iDims ); break;
    case Alembic::Util::kInt16POD:
        return new TypedSampleStore<Util::int16_t>( iDataType, iDims ); break;
    case Alembic::Util::kUint32POD:
        return new TypedSampleStore<Util::uint32_t>( iDataType, iDims ); break;
    case Alembic::Util::kInt32POD:
        return new TypedSampleStore<Util::int32_t>( iDataType, iDims ); break;
    case Alembic::Util::kUint64POD:
        return new TypedSampleStore<Util::uint64_t>( iDataType, iDims ); break;
    case Alembic::Util::kInt64POD:
        return new TypedSampleStore<Util::int64_t>( iDataType, iDims ); break;
    case Alembic::Util::kFloat16POD:
        return new TypedSampleStore<Util::float16_t>( iDataType, iDims ); break;
    case Alembic::Util::kFloat32POD:
        return new TypedSampleStore<Util::float32_t>( iDataType, iDims ); break;
    case Alembic::Util::kFloat64POD:
        return new TypedSampleStore<Util::float64_t>( iDataType, iDims ); break;
    case Alembic::Util::kStringPOD:
        return new TypedSampleStore<Util::string>( iDataType, iDims ); break;
    case Alembic::Util::kWstringPOD:
        return new TypedSampleStore<Util::wstring>( iDataType, iDims ); break;
    default:
        ABCA_THROW( "Unknown datatype in BuildSampleStore: " << iDataType );
        break;
    }
    return NULL;
}


} // End namespace ALEMBIC_VERSION_NS
} // End namespace AbcCoreOgawa
} // End namespace Alembic