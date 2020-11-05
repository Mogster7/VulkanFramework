//------------------------------------------------------------------------------
//
// File Name:	RenderingDefines.h
// Author(s):	Jonathan Bourim (j.bourim)
// Date:        6/24/2020 
//
//------------------------------------------------------------------------------
#pragma once

constexpr size_t MAX_FRAME_DRAWS = 2;

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};



template <typename T>
class HasInitialization
{
    typedef char one;
    struct two { char x[2]; };

    template <typename C> constexpr static one test(decltype(&C::Initialization));
    template <typename C> constexpr static two test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

template <typename T>
constexpr void CheckInitialization(T& obj)
{
    if constexpr (HasInitialization<T>::value)
        obj.Initialization();
}


#define CUSTOM_VK_DECLARE_FULL(myName, vkName, ownerName, derived, EXT) \
    class ownerName;\
    class myName : public eastl::safe_object derived \
    { \
    public:\
        myName() = default;\
        ~##myName() = default;\
        void Destroy();\
        operator vk::vkName##EXT () { return Get(); }\
    \
    protected:\
        eastl::safe_ptr<ownerName> m_Owner = {};\
    private:



#define CUSTOM_VK_CREATE(myName, vkName, ownerName, EXT) \
    public:\
        template <class ...Args>\
        void Create(const vk::vkName##CreateInfo##EXT & createInfo, ownerName& owner, Args&&... args)\
        {\
            m_Owner = &owner; \
            utils::CheckVkResult(m_Owner->create##vkName##EXT(args... , &createInfo, nullptr, &m_##myName), \
                eastl::string("Failed to construct ") + #vkName );\
            CheckInitialization<myName>(*this);\
        }\
    private:\

#define CUSTOM_VK_DERIVED_CREATE_FULL(myName, vkName, ownerName, EXT, createName, constructName) \
    public:\
        template <class ...Args>\
        static void Create(myName& obj, const vk::createName##CreateInfo##EXT & createInfo, \
            ownerName& owner, Args&&... args)\
        {\
            utils::CheckVkResult(owner.create##constructName##EXT(args... , &createInfo, nullptr, &obj), \
                eastl::string("Failed to construct ") + #vkName );\
            obj.m_Owner = &owner; \
            CheckInitialization<myName>(obj);\
        }\
        template <class ...Args>\
        static void Create(myName*&& obj, const vk::createName##CreateInfo##EXT & createInfo, \
            ownerName& owner, Args &&... args)\
        {\
            Create(*obj, createInfo, owner, std::forward(args)...);\
        }\
    private:\

#define CUSTOM_VK_DERIVED_CREATE(myName, vkName, ownerName, EXT) CUSTOM_VK_DERIVED_CREATE_FULL(myName, vkName, ownerName, EXT, vkName, vkName)



#define MEMBER_GETTER(myName, vkName, EXT)\
public:\
    inline vk::vkName##EXT Get() const { return static_cast<vk::vkName##EXT>(m_##myName); }\
private:\
    vk::vkName##EXT m_##myName = {};

#define DERIVED_GETTER(myName, vkName, EXT)\
public:\
    inline vk::vkName##EXT & Get() { return *this; }\
private:

#define CONCAT_COMMA(name) ,##name

#define CUSTOM_VK_DECLARE(myName, vkName, ownerName) \
    CUSTOM_VK_DECLARE_FULL(myName, vkName, ownerName, , ) CUSTOM_VK_CREATE(myName, vkName, ownerName, ) MEMBER_GETTER(myName, vkName, )

#define CUSTOM_VK_DECLARE_NO_CREATE(myName, vkName, ownerName)\
    CUSTOM_VK_DECLARE_FULL(myName, vkName, ownerName, , ) MEMBER_GETTER(myName, vkName, )

#define CUSTOM_VK_DECLARE_KHR(myName, vkName, ownerName) \
    CUSTOM_VK_DECLARE_FULL(myName, vkName, ownerName, , KHR) CUSTOM_VK_CREATE(myName, vkName, ownerName, KHR) MEMBER_GETTER(myName, vkName, KHR)

#define CUSTOM_VK_DECLARE_DERIVE(myName, vkName, ownerName) \
    CUSTOM_VK_DECLARE_FULL(myName, vkName, ownerName, CONCAT_COMMA(public vk::##vkName) , ) CUSTOM_VK_DERIVED_CREATE(myName, vkName, ownerName, ) DERIVED_GETTER(myName, vkName, )

#define CUSTOM_VK_DECLARE_DERIVE_NO_CREATE(myName, vkName, ownerName) \
    CUSTOM_VK_DECLARE_FULL(myName, vkName, ownerName, CONCAT_COMMA(public vk::##vkName) , ) DERIVED_GETTER(myName, vkName, ) 

#define CUSTOM_VK_DECLARE_DERIVE_KHR(myName, vkName, ownerName) \
    CUSTOM_VK_DECLARE_FULL(myName, vkName, ownerName, CONCAT_COMMA(public vk::##vkName##KHR) , KHR) CUSTOM_VK_DERIVED_CREATE(myName, vkName, ownerName, KHR) DERIVED_GETTER(myName, vkName, KHR)

#define CUSTOM_VK_DEFINE_FULL(myName, vkName, ownerName, EXT)\
    void myName::Destroy()\
    {\
        m_Owner->destroy##vkName##EXT(Get());\
    }

#define CUSTOM_VK_DEFINE(myName, vkName, ownerName) CUSTOM_VK_DEFINE_FULL(myName, vkName, ownerName,)
#define CUSTOM_VK_DEFINE_KHR(myName, vkName, ownerName) CUSTOM_VK_DEFINE_FULL(myName, vkName, ownerName, KHR)


