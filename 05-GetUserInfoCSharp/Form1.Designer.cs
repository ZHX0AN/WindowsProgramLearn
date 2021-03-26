
namespace _05_GetUserInfoCSharp
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.button1 = new System.Windows.Forms.Button();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.BTN_UTF8 = new System.Windows.Forms.Button();
            this.Emoji_TEST = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(12, 12);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 0;
            this.button1.Text = "确定";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(12, 41);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(297, 374);
            this.textBox1.TabIndex = 1;
            // 
            // BTN_UTF8
            // 
            this.BTN_UTF8.Location = new System.Drawing.Point(122, 13);
            this.BTN_UTF8.Name = "BTN_UTF8";
            this.BTN_UTF8.Size = new System.Drawing.Size(75, 23);
            this.BTN_UTF8.TabIndex = 2;
            this.BTN_UTF8.Text = "UTF8测试";
            this.BTN_UTF8.UseVisualStyleBackColor = true;
            this.BTN_UTF8.Click += new System.EventHandler(this.BTN_UTF8_Click);
            // 
            // Emoji_TEST
            // 
            this.Emoji_TEST.Location = new System.Drawing.Point(412, 96);
            this.Emoji_TEST.Name = "Emoji_TEST";
            this.Emoji_TEST.Size = new System.Drawing.Size(75, 23);
            this.Emoji_TEST.TabIndex = 3;
            this.Emoji_TEST.Text = "Emoji";
            this.Emoji_TEST.UseVisualStyleBackColor = true;
            this.Emoji_TEST.Click += new System.EventHandler(this.Emoji_TEST_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 17F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(706, 543);
            this.Controls.Add(this.Emoji_TEST);
            this.Controls.Add(this.BTN_UTF8);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.button1);
            this.Name = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Button BTN_UTF8;
        private System.Windows.Forms.Button Emoji_TEST;
    }
}

