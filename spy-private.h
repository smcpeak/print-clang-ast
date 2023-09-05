// spy-private.h
// Provide the means to spy on private fields.

// This is a header-only module.

// This is based on:
//
//   http://bloglitb.blogspot.com/2011/12/access-to-private-members-safer.html
//
// but I've added extensive comments, renamed all of the entities for
// clarity and collision resistance, and added macros to make usage more
// convenient.

#ifndef SPY_PRIVATE_H
#define SPY_PRIVATE_H


// This template is given the pointer-to-member we want as its template
// non-type argument 'PTR_TO_MEMBER'.  Its purpose is then to expose a
// friend function that will return that pointer.
//
// It also needs the 'BRIDGE' type for the signature of 'getPrivatePTM'.
template <class BRIDGE,
          typename BRIDGE::ptmType PTR_TO_MEMBER>
struct PrivatePTMGetter {
  // This will define the 'getPrivatePTM' method that the bridge
  // declares.  The parameter value is not used; the only purpose of the
  // parameter is to match the signature of the function declared by
  // 'PrivateADLBridge'.
  inline friend typename BRIDGE::ptmType getPrivatePTM(BRIDGE) {
    return PTR_TO_MEMBER;
  }
};


// Define a bridge that can be used in an explicit instantiation of
// 'PrivatePTMGetter' to expose the private field
// 'CLASSTYPE::FIELDTYPE'.
//
// 'discrim' can be anything, but if there are two private fields we
// want to access that have the same type and are members of the same
// class, then they need distinct discriminators (otherwise the
// generated 'getPrivatePTM' functions will collide).
//
// Note that the client needs to be able to name 'FIELDTYPE'.  If that
// is private as well, then this method won't work.  It also does not
// work for bitfields since they can't form a pointer-to-member.
template <class CLASSTYPE, class FIELDTYPE, int discrim>
struct PrivateADLBridge {
  // This typedef offers the minor convenience of not needing to
  // separately pass the field type to 'PrivatePTMGetter', and elsewhere
  // avoid using the pointer-to-member declarator syntax.
  typedef FIELDTYPE CLASSTYPE::*ptmType;

  // We declare a function that can be found by argument-dependent
  // lookup (ADL) for a bridge-typed argument.  Because it is a friend
  // rather than a method, 'PrivatePTMGetter' can then define it despite
  // being an unrelated class template.
  //
  // This forms a "bridge" from the client, who can name the bridge
  // class, to the getter class, which the client cannot name (outside
  // of explicit instantiations), due to the access control
  // restrictions.
  //
  // GCC warns about this: "friend declaration [...] declares a
  // non-template function".  Silence with -Wno-non-template-friend.
  // Clang is fine with it.
  inline friend ptmType getPrivatePTM(PrivateADLBridge);
};


// Enable the use of 'ACCESS_PRIVATE_FIELD' for a particular field of a
// particular class.
//
// This works by explicitly instantiating 'PrivatePTMGetter', passing a
// pointer-to-member to the private member of interest; that is allowed
// because template arguments passed during explicit instantiation are
// exempt from normal access control rules.  The effect is to define the
// 'getPrivatePTM' friend function such that it returns a pointer to the
// private member.  We can then look up 'getPrivatePTM' via ADL and the
// bridge class.
//
// If 'ClassType' or 'FieldType' is itself a template-id (a template
// name followed by template arguments in angle brackets), then define a
// typedef first in order to avoid the problem of commas splitting up
// macro arguments.
//
// This must be used at file or namespace scope.
#define ENABLE_ACCESS_PRIVATE_FIELD(ClassType, FieldType, FieldName, discrim)         \
  template struct PrivatePTMGetter<PrivateADLBridge<ClassType, FieldType, (discrim)>, \
                                   &ClassType::FieldName>

// After 'ENABLE_ACCESS_PRIVATE_FIELD' with the same arguments has been
// used, this will get the indicated field.  Since this is a dereference
// of a pointer-to-member, the constness of the result tracks that of
// 'obj'.
#define ACCESS_PRIVATE_FIELD(obj, ClassType, FieldType, discrim) \
  ((obj).*(getPrivatePTM(PrivateADLBridge<ClassType, FieldType, (discrim)>())))


#endif // SPY_PRIVATE_H
