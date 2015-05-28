rm -f stack_and_service/ipaugenblick_service_build.h
echo '#ifndef __IPAUGENBLICK_SERVICE_BUILD_H__' > stack_and_service/ipaugenblick_service_build.h
echo '#define __IPAUGENBLICK_SERVICE_BUILD_H__' >> stack_and_service/ipaugenblick_service_build.h
echo '#define IPAUGENBLICK_SERVICE_BUILD \' >> stack_and_service/ipaugenblick_service_build.h
git log -1 --pretty=oneline | gawk '{print "\"", $1, "\"" }' >> stack_and_service/ipaugenblick_service_build.h
echo '#define IPAUGENBLICK_SERVICE_UNTRACKED \' >> stack_and_service/ipaugenblick_service_build.h
git status --porcelain | wc | gawk '{print "\"", $1, "\"" }' >> stack_and_service/ipaugenblick_service_build.h
echo '#endif /* __IPAUGENBLICK_SERVICE_BUILD_H__ */' >> stack_and_service/ipaugenblick_service_build.h
