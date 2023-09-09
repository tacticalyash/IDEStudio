#include <string>
#include <vector>
#include <set>

struct KMXProperty
{
    std::string name;
    std::string value;
    bool operator < (const KMXProperty &rhs) const;   //for std::set
};

inline bool KMXProperty::operator <(const KMXProperty &rhs) const
{
    return name < rhs.name;
}

class KMXTag
{
public:
    enum TagType{
        Container,Leaf
    };
public:
    KMXTag(const std::string &tag_name);
    const std::string &getTagName() const;
    void insertProperty(const KMXProperty &property);
    const std::set<KMXProperty> &getProperties();
    void addChildTag(KMXTag *tag);
    template <class Stream>
    void writeTo(Stream &stream, int spaces, int mul) const;
    ~KMXTag();
    TagType tagType() const;
    void setTagType(TagType newType);

private:
    std::string m_tag_name;
    std::set<KMXProperty> m_properties;
    std::vector<KMXTag*> m_children_tags;
    TagType m_type;
};

/**
 * tabs -> number of tabs
 * spaces -> number of spaces in tabs
 */
template<class Stream>
void KMXTag::writeTo(Stream &stream, int tabs, int spaces) const
{
    stream << std::string(tabs * spaces, ' ');
    stream << "<" << m_tag_name;
    for(const KMXProperty &property : m_properties)
        stream << '\n' << std::string((tabs+1) * spaces, ' ') << property.name << " = \"" << property.value << "\"";
    if(m_type == TagType::Container)
    {
        stream << ">\n";
        for(KMXTag *child_tag : m_children_tags)
        {
            child_tag->writeTo(stream,tabs+1,spaces);
        }
        stream << std::string(tabs * spaces, ' ') << "</" << m_tag_name << ">";
    }
    else        //leaf tag
    {
        stream << "/>";
    }
    stream << "\n";
    stream.flush();
}


inline KMXTag::KMXTag(const std::string &tag_name) : m_tag_name(tag_name), m_type(Container)
{
    //
}

inline const std::string &KMXTag::getTagName() const
{
    return m_tag_name;
}

inline void KMXTag::insertProperty(const KMXProperty &property)
{
    m_properties.insert(property);
}

inline const std::set<KMXProperty> &KMXTag::getProperties()
{
    return m_properties;
}

inline void KMXTag::addChildTag(KMXTag *tag)
{
    m_children_tags.push_back(tag);
}

inline KMXTag::TagType KMXTag::tagType() const
{
    return m_type;
}

inline void KMXTag::setTagType(TagType newType)
{
    m_type = newType;
}

KMXTag *toKMXTag(const std::string& str, std::string &error);
