import java.lang.reflect.*;

public class Inspector {
	
	// false or true to do recursion
    public void inspect(Object obj, boolean recursive) {
        inspectClass(obj.getClass(), obj, recursive, 0);
    }
    // indentation by depth
    public String Indent(int depth) {
        StringBuilder indent = new StringBuilder();
        for (int i = 0; i < depth; i++) {
            indent.append("\t");
        }
        return indent.toString();
    }
    
    // Inspect details of objects
    public void inspectClass(Class<?> classObj, Object obj, boolean recursive, int depth) {
        String indent = Indent(depth);
        className(indent, classObj);
        if (classObj.isArray()) {
            Array(indent, obj);
            return;
        }
        declaringClass(indent, classObj, depth);
        superClass(indent, classObj, obj, recursive, depth);
        interfaces(indent, classObj, obj, recursive, depth);
        methods(indent, classObj, depth);
        Constructors(indent, classObj, depth);
        Fields(indent, classObj, obj, recursive, depth);
    }

    //printing name of class
    public void className(String indent, Class<?> classObj) {
        System.out.println(indent + classObj.getName());
    }
    
    //prints array
    public void Array(String indent, Object array) {
        System.out.println(indent + getArray(array));
    }

    //declares class
    public void declaringClass(String indent, Class<?> classObj, int depth) {
        System.out.println(indent + "Declaring Class: " + classObj.getName());
    }

    //print superclass 
    public void superClass(String indent, Class<?> classObj, Object obj, boolean recursive, int depth) {
        Class<?> superClass = classObj.getSuperclass();
        if (superClass != null) {
            System.out.println(indent + "Immediate Superclass: " + superClass.getName());
            inspectClass(superClass, obj, recursive, depth + 1);
        }
    }

    //print interfaces
    public void interfaces(String indent, Class<?> classObj, Object obj, boolean recursive, int depth) {
        for (Class<?> interfaces : classObj.getInterfaces()) {
            System.out.println(indent + "Interfaces: " + interfaces.getName());
            inspectClass(interfaces, obj, recursive, depth + 1);
        }
    }
    
    //print methods
    public void methods(String indent, Class<?> classObj, int depth) {
        System.out.println(indent + "Methods: ");
        for (Method method : classObj.getDeclaredMethods()) {
            System.out.println(indent + getMethodData(method, depth));
        }
    }

    //prints constructors
    public void Constructors(String indent, Class<?> classObj, int depth) {
        System.out.println(indent + "Constructors: ");
        for (Constructor<?> constructor : classObj.getDeclaredConstructors()) {
            System.out.println(indent + getConstructorData(constructor, depth));
        }
    }

    //prints field
    public void Fields(String indent, Class<?> classObj, Object obj, boolean recursive, int depth) {
        System.out.println(indent + "Fields: ");
        for (Field field : classObj.getDeclaredFields()) {
            System.out.println(indent + getFieldData(obj, field, recursive, depth));
        }
    }
    
    //get data of method
    public String getMethodData(Method method, int depth) {
        StringBuilder info = new StringBuilder(Indent(depth));
        info.append(Modifier.toString(method.getModifiers())).append(" ");
        info.append(method.getReturnType().getName()).append(" ");
        info.append(method.getName()).append("(");
        info.append(joinClassNames(method.getParameterTypes())).append(")");

        if (method.getExceptionTypes().length > 0) {
            info.append("Throw ").append(joinClassNames(method.getExceptionTypes()));
        }

        return info.toString();
    }

    //get constructor data
    public String getConstructorData(Constructor<?> constructor, int depth) {
        StringBuilder info = new StringBuilder(Indent(depth));

        info.append(Modifier.toString(constructor.getModifiers())).append(" ");
        info.append(constructor.getName()).append("(");
        info.append(joinClassNames(constructor.getParameterTypes())).append(")");

        return info.toString();
    }

    //get field data
    public String getFieldData(Object obj, Field field, boolean recursive, int depth) {
        StringBuilder info = new StringBuilder(Indent(depth));
        String valueStr = getValueString(obj, field, recursive, depth);

        info.append(Modifier.toString(field.getModifiers())).append(" ");
        info.append(field.getType().getName()).append(" ");
        info.append(field.getName()).append(" = ");
        info.append(valueStr);

        return info.toString();
    }

    //get value of field, if field value object, do recursion
    public String getValueString(Object obj, Field field, boolean recursive, int depth) {
        try {
            field.setAccessible(true);
            Object value = field.get(obj);
            if (value == null) return "null";
            if (field.getType().isPrimitive()) return value.toString();
            if (field.getType().isArray()) return getArray(value);
            if (recursive) inspectClass(value.getClass(), value, true, depth + 1);
            return value.toString();
        } 
        catch (IllegalAccessException e) {
            return "IllegalAccess";
        }
    }

    //get array data
    public String getArray(Object array) {
        int length = Array.getLength(array);
        StringBuilder data = new StringBuilder("[Length: " + length + "] -> ");
        for (int i = 0; i < length; i++) {
            data.append(Array.get(array, i));
            if (length > i) {
                data.append(", ");
            }
        }
        return data.toString();
    }

    //join class
    public String joinClassNames(Class[] classes) {
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < classes.length; i++) {
            result.append(classes[i].getName());
            if (i < classes.length - 1) {
                result.append(", ");
            }
        }
        return result.toString();
    }
}
