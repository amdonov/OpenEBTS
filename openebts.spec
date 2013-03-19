Name:   OpenEBTS     
Version: 2.0.1       
Release:        1%{?dist}
Summary:      The OpenEBTS library simplifies the programmtic reading, writing, and editing of EBTS files.

License: Apache 2.0        
URL: https://github.com/amdonov/OpenEBTS
Source0: %{name}-%{version}.tar.gz

BuildRequires: NBIS freeimage-devel libcurl-devel
Requires: freeimage NBIS libcurl     

%description


%prep
%setup -q


%build
cmake . -DCMAKE_INSTALL_PREFIX=$RPM_BUILD_ROOT
make

%install
rm -rf $RPM_BUILD_ROOT
make install

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/*
%doc

%post
/sbin/ldconfig

%postun
/sbin/ldconfig



%changelog
* Tue Mar 19 2013 Aaron Donovan <amdonov@gmail.com> 2.0.1-1
- new package built with tito

