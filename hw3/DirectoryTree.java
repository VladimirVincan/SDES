import java.io.File;
import java.nio.file.InvalidPathException;

public class DirectoryTree
{
    public Node root;
    public DirectoryTree(String filepath)
    {
        root = new Node(null, filepath);
    }

    public void populateTree()
    {
        if (! root.representsFile())
            root.createChildren();
        else
            // System.out.println("InvalidPathException");
            throw new InvalidPathException(root.filename, "It is not a directory.");
    }

    public void print_tree()
    {
        root.print_tree();
    }

    public Node findMaxSizeDirectory(Node node)
    {
        if (node == null)
            return null;

        if (node.representsFile())
            return null;

        if (node.children == null)
            return node;

        Node maxn = node;

        for (Node child : node.children)
            {
                Node curr = findMaxSizeDirectory(child);
                if (curr != null)
                    if (maxn == null || curr.calculateSize() > maxn.calculateSize())
                        maxn = curr;
            }
        return maxn;
    }

    public Node findMaxSizeFile(Node node)
    {
        if (node == null)
            return null;

        // System.out.println(node.filename);

        if (node.representsFile())
            return node;

        if (node.children == null)
            return null;

        Node maxn = null;

        for (Node child : node.children)
            {
                Node curr = findMaxSizeFile(child);
                if (maxn == null || curr.calculateSize() > maxn.calculateSize())
                    maxn = curr;
            }
        return maxn;
    }
}
