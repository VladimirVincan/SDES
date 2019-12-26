public class DirectoryTreeTest
{
    public static void main(String[] args)
    {
        String path = args[0];


        // Node node = new Node(null, path);
        // node.createChildren();
        // node.print_tree();
        DirectoryTree tree = new DirectoryTree(path);

        System.out.println("**********************************************");
        System.out.println("Files from main directory : " + path);
        System.out.println("**********************************************");

        tree.populateTree();
        tree.print_tree();

        System.out.println("**********************************************");
        System.out.println("Max Size Directory:");
        System.out.println("**********************************************");
        System.out.println(tree.findMaxSizeDirectory(tree.root).filename);
        System.out.println("size = " + (tree.findMaxSizeDirectory(tree.root)).size);

        System.out.println("**********************************************");
        System.out.println("Max Size File:");
        System.out.println("**********************************************");
        System.out.println(tree.findMaxSizeFile(tree.root).filename);
        System.out.println("size = " + (tree.findMaxSizeFile(tree.root)).size);
    }
}
